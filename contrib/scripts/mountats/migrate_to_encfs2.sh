#!/bin/bash
# CONFIG
CHECKHOST=equalit.ie

source /root/.bashrc
source /usr/local/deflect/scripts/encfs_pass2
shopt -s expand_aliases
export TERM=vt100
#set -e

HOSTS=${1}" "

if [ -z "${HOSTS}" ]; then
 echo "Error: Need to supply hosts as arg"
 exit
fi


function migrate_encfs() {
    host=$@

    #setup
    ssh $host "apt-get install -y encfs"

    # Stop ATS and create mount"
    ssh $host "/etc/init.d/trafficserver stop;
     service log-courier stop;
     mv /usr/local/trafficserver /usr/local/trafficserver.migrating;
     mkdir /usr/local/trafficserver /usr/local/.trafficserver;"
     /usr/local/deflect/scripts/init-enc.exp $host $ENCFS_PASS /usr/local/.trafficserver/ /usr/local/trafficserver/

     # this sleep is required for the mount to finish
     sleep 2
     ssh $host grep /usr/local/trafficserver /proc/mounts
     if [ $? -ne 0 ]; then 
       echo 'Migration failed!'
       ssh $host "rmdir /usr/local/trafficserver;
       mv /usr/local/trafficserver.migrating /usr/local/trafficserver; 
	 service log-courier start;
       /etc/init.d/trafficserver start"
     else
	 ssh $host "shopt -s dotglob; mv /usr/local/trafficserver.migrating/* /usr/local/trafficserver/;
         chown -R trafserv:trafserv /usr/local/trafficserver/;
	 /etc/init.d/trafficserver start;
	 service log-courier start;
	 rmdir /usr/local/trafficserver.migrating"
	 echo "Finished migrating $host"
	 curl -H "Host: $CHECKHOST" $host > /dev/null
	 if [ $? -ne 0 ]; then 
	     echo "Couldn't get page from $host - check setup"
	 fi
     fi
}

for host in $HOSTS; do 
    echo $host
    ssh $host grep /usr/local/trafficserver /proc/mounts > /dev/null
    if [ $? -ne 0 ]; then 
        echo "No encfs on $host- migrating"
	migrate_encfs $host 
    fi
done

