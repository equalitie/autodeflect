#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/time.h>
#include "conf.h"
#include "cglobals.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <libssh2.h>

#include <openssl/md5.h>

/* "runner" daemon */

int get_ip(char *  , char *);
int open_ssh(char * , char * , char *, int , char *);
static int waitsocket(int , LIBSSH2_SESSION *);
int md5sum_check(char * , unsigned char *);

int main(int argc, char **argv)
{

	unsigned char digest[MD5_DIGEST_LENGTH] = {0};
	char *remote_md5 = (char*)malloc(33);
	char *local_md5 = (char*)malloc(33);
	int i = 0;
	struct timeval tv;
	char etime[21];

	double current_load;

	lib_common_option_handling(argc, argv);

	if (!handle_pid_file_checks(pid_runner, PROGRAM_NAME_RUNNER)) {
		fprintf(stderr, "handle_pid_file_checks indicates exit required\n");
		exit(EXIT_FAILURE);
	}

	populate_globals();

	while (1) {
		if (need_finish == TRUE)
			break;

		if (load_check(&current_load)) {
			if (current_load > max_load) {
				fprintf(stderr, "Load too high\n");
				sleep(daemon_interval_high_load);
				continue;
			}
		} else {
			fprintf(stderr, "Unable to check load\n");
			sleep(daemon_interval_generic);
			continue;
		}

		if (access(process_file, F_OK) == 0) {
			fprintf(stderr, "%s file already exists\n", process_file);
			sleep(daemon_interval_generic);
			continue;
		}

		/* FIXME: Should we make this configurable? */
		char outfile[] = "/tmp/autodeflectXXXXXX";

		if (!open_ssh(dashboard_host, dashboard_user, dashboard_client_yml, dashboard_port, outfile)) {
			fprintf(stderr, "Could not get remote file\n");
			sleep(daemon_interval_generic);
			continue;
		}

		/* Clear content */
		remote_md5[0] = 0;
		local_md5[0] = 0;
		digest[0] = 0;

		if (md5sum_check(outfile, digest)) {

			for (i = 0; i < MD5_DIGEST_LENGTH; ++i) {
				snprintf(&(remote_md5[i*2]), 16*2, "%02x", (unsigned int)digest[i]);
			}
			fprintf(stderr, "remote_md5: %s\n", remote_md5);

			unlink(outfile);
		}

		digest[0] = 0;

		if (md5sum_check(last_clients_yml, digest)) {

			for (i = 0; i < MD5_DIGEST_LENGTH; ++i) {
				snprintf(&(local_md5[i*2]), 16*2, "%02x", (unsigned int)digest[i]);
			}
			fprintf(stderr, "local_md5: %s\n", local_md5);
		}


		if ( (strlen(local_md5) == 32)  & (strlen(remote_md5) == 32) ) {
			if ( (strcmp(local_md5, remote_md5) == 0) ) {
				fprintf(stderr, "Local and Remote are same files\n");
			} else {
				fprintf(stderr, "Local and Remote are different files\n");
				int processfd = open(process_file, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
				if (processfd != -1) {
					gettimeofday(&tv, NULL);
					unsigned long long epochtime = 
						(unsigned long long)(tv.tv_sec) * 1000 +
						(unsigned long long)(tv.tv_usec) / 1000;
					size_t etimeSize = snprintf(NULL, 0 , "%llu", epochtime);

					snprintf(etime, etimeSize + 1, "%llu", epochtime);
					write(processfd, etime, etimeSize + 1);

					close(processfd);
				} else {
					fprintf(stderr, "Problem writting %s\n", process_file);
				}
			}
		} else {
			fprintf(stderr, "Problem getting md5sum of files\n");
		}

		sleep(daemon_interval_generic);
	}

	return EXIT_SUCCESS;
}

/***************************************************************************************
***************************************************************************************/
 
int get_ip(char * hostname , char* ip)
{
	struct hostent *he;
	struct in_addr **addr_list;

	if ((he = gethostbyname(hostname)) == NULL) {
		fprintf(stderr, "Lookup failed for %s\n", hostname);
		return FALSE;
	}

	addr_list = (struct in_addr **) he->h_addr_list;
	if (addr_list[0] != NULL) {
		strcpy(ip , inet_ntoa(*addr_list[0]));
		return TRUE;
	}

	return FALSE;
}

/***************************************************************************************
***************************************************************************************/

int open_ssh(char * hostname, char * username , char * remotefile, int port, char * outfile) {

	unsigned long hostaddr;
	int sock = -1, i, rc, tmpfile = 0;
	char ip[100];
	struct sockaddr_in sin;
	const char *finprint;
	char *userauthlist;
	LIBSSH2_SESSION *session = NULL;
	LIBSSH2_CHANNEL *channel = NULL;
	LIBSSH2_AGENT *agent = NULL;
	struct libssh2_agent_publickey *identity, *prev_identity = NULL;
	struct stat fileinfo;
	int spin = 0;
	size_t got = 0;
	size_t total = 0;
	/* debian 7 does not have this in libssh2 yet */
	/***********************************
	libssh2_struct_stat_size got = 0;
	libssh2_struct_stat_size total = 0;
	************************************/

	if (!get_ip(hostname , ip)) {
		return FALSE;
	}

	hostaddr = inet_addr(ip);

	rc = libssh2_init (0);
	if (rc != 0) {
		fprintf(stderr, "libssh2 initialization failed (%d)\n", rc);
		return FALSE;
	}

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		fprintf(stderr, "failed to create socket!\n");
		rc = 0;
		goto end;
	}

	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = hostaddr;
	if (connect(sock, (struct sockaddr*)(&sin),
			sizeof(struct sockaddr_in)) != 0) {
		fprintf(stderr, "failed to connect!\n");
		rc = 0;
		goto end;
	}

	session = libssh2_session_init();
	if (libssh2_session_handshake(session, sock)) {
		fprintf(stderr, "Failure establishing SSH session\n");
		rc = 0;
		goto end;
	}

	finprint = libssh2_hostkey_hash(session, LIBSSH2_HOSTKEY_HASH_SHA1);
	fprintf(stderr, "Fingerprint: ");
	for (i = 0; i < 20; i++) {
		if (i == 0) {
			fprintf(stderr, "%02X", (unsigned char)finprint[i]);
		} else {
			fprintf(stderr, ":%02X", (unsigned char)finprint[i]);
		}
	}
	fprintf(stderr, "\n");

	userauthlist = libssh2_userauth_list(session, username, strlen(username));
	fprintf(stderr, "Authentication methods available: %s\n", userauthlist);
	if (strstr(userauthlist, "publickey") == NULL) {
		fprintf(stderr, "\"publickey\" authentication is not supported\n");
		rc = 0;
		goto end;
	}

	agent = libssh2_agent_init(session);
	if (!agent) {
		fprintf(stderr, "Failure initializing ssh-agent support\n");
		rc = 0;
		goto end;
	}

	if (libssh2_agent_connect(agent)) {
		fprintf(stderr, "Failure connecting to ssh-agent\n");
		rc = 0;
		goto end;
	}

	if (libssh2_agent_list_identities(agent)) {
		fprintf(stderr, "Failure requesting identities to ssh-agent\n");
		rc = 0;
		goto end;
	}

	while (1) {
		rc = libssh2_agent_get_identity(agent, &identity, prev_identity);
		if (rc == 1) {
			rc = 0;
			break;
		}

		if (rc < 0) {
			fprintf(stderr, "Failure obtaining identity from ssh-agent support\n");
			rc = 0;
			goto end;
		}

		if (libssh2_agent_userauth(agent, username, identity)) {
			fprintf(stderr, "Authentication with username %s and "
				"public key %s failed!\n",
				username, identity->comment);
		} else {
			fprintf(stderr, "Authentication with username %s and "
				"public key %s succeeded!\n",
				username, identity->comment);
			rc = 1;
			break;
		}

		prev_identity = identity;
	}

	if (!rc) {
		fprintf(stderr, "Couldn't continue authentication\n");
		goto end;
	}

	/* Set non-blocking */
	libssh2_session_set_blocking(session, 0);

	do {

		/* This function is DEPRECATED. Use libssh2_scp_recv2 instead! */
		/* not in Debian 7 so used */
		channel = libssh2_scp_recv(session, remotefile, &fileinfo);

		if (!channel) {
			if(libssh2_session_last_errno(session) != LIBSSH2_ERROR_EAGAIN) {
				fprintf(stderr, "Unable to open a session\n");
				rc = 0;
				goto end;
			} else {

				waitsocket(sock, session);
			}
		}
	} while (!channel);

	tmpfile = mkstemp(outfile);

	while(got < fileinfo.st_size) {
	char mem[1024*24];

		do {
			int amount = sizeof(mem);

			if ((fileinfo.st_size -got) < amount) {
				amount = (int)(fileinfo.st_size - got);
			}

			/* loop until we block */ 
			rc = libssh2_channel_read(channel, mem, amount);

			if (rc > 0) {
				write(tmpfile, mem, rc);
				got += rc;
				total += rc;
			}
		} while (rc > 0);

		if ((rc == LIBSSH2_ERROR_EAGAIN) && (got < fileinfo.st_size)) {
		/* this is due to blocking that would occur otherwise
		so we loop on this condition */ 

			spin++;
			waitsocket(sock, session); /* now we wait */ 
			continue;
		}
		break;
	}

	int size = fileinfo.st_size;

	fprintf(stderr, "File size: %d\n", size);


end:

	if (tmpfile > 0) {
		close(tmpfile);
	}

	if (channel) {
		libssh2_channel_free(channel);
		channel = NULL;
	}

	if (agent) {
		libssh2_agent_disconnect(agent);
		libssh2_agent_free(agent);
	}

	if (session) {
		libssh2_session_disconnect(session, "Shutting down session");
		libssh2_session_free(session);
	}

	if (sock != -1) {
		close(sock);
	}

	libssh2_exit();

	return rc;
}

/***************************************************************************************
***************************************************************************************/

static int waitsocket(int socket_fd, LIBSSH2_SESSION *session)
{
	struct timeval timeout;
	int rc;
	fd_set fd;
	fd_set *writefd = NULL;
	fd_set *readfd = NULL;
	int dir;
 
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;

	FD_ZERO(&fd);

	FD_SET(socket_fd, &fd);

	/* now make sure we wait in the correct direction */ 
	dir = libssh2_session_block_directions(session);

	if(dir & LIBSSH2_SESSION_BLOCK_INBOUND)
		readfd = &fd;

	if(dir & LIBSSH2_SESSION_BLOCK_OUTBOUND)
		writefd = &fd;
 
	rc = select(socket_fd + 1, readfd, writefd, NULL, &timeout);
 
	return rc;
}

/***************************************************************************************
***************************************************************************************/

int md5sum_check(char * filename, unsigned char c[MD5_DIGEST_LENGTH]) {
	FILE *inFile = fopen (filename, "rb");
	MD5_CTX mdContext;
	int bytes;
	unsigned char data[1024];

	if (inFile == NULL) {
		fprintf (stderr, "%s can't be opened.\n", filename);
		return FALSE;
	}

	MD5_Init (&mdContext);
	while ((bytes = fread (data, 1, 1024, inFile)) != 0) {
		MD5_Update (&mdContext, data, bytes);
	}
	MD5_Final (c,&mdContext);

	if (inFile)
		fclose (inFile);

	return TRUE;
}
