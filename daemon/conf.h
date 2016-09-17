/******************************************************************************/
/******************************************************************************/

#ifndef _conf_h_
#define _conf_h_

#define	FALSE	0
#define TRUE	1

#define MODE_NORMAL 10

/**********************************************************************/
// Program Name Definitions
/**********************************************************************/

#define PROGRAM_NAME_PROCESS "ad2p"
#define PROGRAM_NAME_SSH_KEY "ad2ssh"
#define PROGRAM_NAME_RUNNER "ad2runner"

/**********************************************************************/
// Processing programs
/**********************************************************************/

#define PROGRAM_PROCESS "autobrains_update.sh"

/**********************************************************************/
// External definitions
/**********************************************************************/

#define MAX_PASS_SIZE 1024

// From loadchk.c
int load_check(double *current_load);

// From pid.c
int handle_pid_file_checks(const char *pid_file, const char *program_name);
// NOTE: The following three functions are mainly used in the pid.c file. They
// are also used in the autodctl program.
int check_pid_file_exists(const char *pid_file, int *exists, pid_t *existing_pid);
int create_pid_file(const char *pid_file);
int check_process_running(pid_t pid, const char *program_name, int *running);

// From signal.c
extern int need_finish;
void finish_handler(int sig);

// lib.c

void lib_common_option_handling(int argc, char **argv);

int populate_globals(void);

// config.c

extern char *sh_command;
extern char *directory_script;
extern char *directory_daemon;
extern char *directory_run;
extern char *pid_suffix;
extern char *program_process;
extern char *process_file;
extern char *pid_process;
extern char *pid_ssh_key;
extern char *pid_runner;
extern char *ssh_agent;
extern char *ssh_key_file;
extern char *ssh_add;
extern char *ssh_key_fingerprint;
extern char *ssh_agent_sock;
extern char *path;
extern char *passphrase;
extern char *dashboard_user;
extern char *dashboard_host; 
extern char *dashboard_client_yml;
extern char *last_clients_yml;
extern int dashboard_port; 

int config_load(char *filename);

/**********************************************************************/
// Globals
/**********************************************************************/

extern double max_load;
extern int daemon_interval_high_load;
extern int daemon_interval_generic;
extern int daemon_interval_process;
extern int daemon_interval_ssh_key;

#endif
