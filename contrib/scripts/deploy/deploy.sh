#!/bin/bash
#
# deflect sysadmin makefile shell script
#
# Most all configuration of this script gets pulled in from common.sh and default conf file etc/deflect.config

##### functions

help() { 
  echo "	edge control - deflect style
usage:
  -b [ hostname ]	  initial build and full sync
  -h			  print this help
  -H [ hostname | ALL ]	  target host (or ALL / NONLIVE / LIVE)
  -i			  ignore ping test - deploy anyway
  -n			  don't really push
  -N			  do Not copy files - restart only
  -l                      'el' Instead of a ats restart, this does a reload
  -p [ package | ALL ]	  package to push
  -P [ command ]	  run an arbitrary remote command
  -s			  sync-only: do no restart anything
  -S [ path ]		  takes an argument to sync any
			  arbitrary path
  -A [ alt_config ]       Use alternate configuration variables from deflect.config
                          Do not use -A to use [default].
                          -A default
                          will return all names in config file.
  -v			  be more verbose (useful!)

Examples:
$MYNAME -b -H ch1		- build ch1.${DEFLECT_DOM}
$MYNAME -H ch1 -s -p ALL	- synchronise edge/ tree
$MYNAME -H ch1 -p fail2ban	- synchronise fail2ban files and
				  restart fail2ban on ch1
$MYNAME -H ch1 -S etc/	- synchronise edge/etc to ch1:/etc
				  [* Note: no leading slash]

Available args to -p are:
ats	  - trafficserver config only
atsconf	  - trafficserver config only
atsmods	  - trafficserver modules only
atsall	  - trafficserver config, modules, binaries (be careful of Debian version mismatches)
fail2ban  - fail2ban config
rsyslog   - rsyslog 
firewall  - iptables script (implies fail2ban)
logcourier - log-courier config files
nagios	  - nrpe nagios client, including its helpers
swabber   - install swabber
conftouch - sync only the file ATS_DIR/conf/dashboard.timestamp
misc	  - currently: $MISC_PATHS
ALL	  - synchronises everything in the tree under
	    $EDGEBASE - WILL restart
	    services unless -s is specified

Each invocation of $MYNAME is logged in $DPLOGFILE

WARNING: this script pushes config and restarts live services.
WARNING: test changes on a non-live edge first
"
}

finish () {
  if [ -f $DNETLOCK ]; then
    rm -rf $DNETLOCK
  fi
}

do_sync() {
  echo "synchronising files in $SYNCPATH to $EDGENAME"
  [ -z "$SYNCPATH" ] && echo "tried to sync but SYNCPATH not set - exiting" && exit 1
  cd $EDGEBASE
  $V rsync -avpzRL -e "ssh" --exclude='old/' ./$SYNCPATH root@${EDGENAME}:/
}

do_restart() {
  [ -z "$RESTART_CMD" ] && echo "RESTART_CMD not set - error" && exit 1
  echo "running $RESTART_CMD on $EDGENAME"
  $V ssh -l root $EDGENAME "$RESTART_CMD"
}

ats_hostname() {
  # insert the target hostname into records.config
  myTemplate=$(mktemp)
  # Don't do DEBUG here, breaks. removed $V prefix
  cat ${EDGEBASE}${TSPATH}/conf/records.config > $myTemplate && echo "inserting $EDGENAME into records.config"
#  [ -n "$V" ] && echo "inserting $EDGENAME into records.config"
  sed -e "s%^CONFIG proxy.config.proxy_name STRING.*%CONFIG proxy.config.proxy_name STRING ${EDGENAME}%" $myTemplate > ${EDGEBASE}${TSPATH}/conf/records.config
 chown $TSUID:$TSGID ${EDGEBASE}${TSPATH}/conf/records.config
 rm $myTemplate
 unset myTemplate
}

ats_unhostname() {
   # set proxy.config.proxy_name to deflect_ats so bvi doesn't flip every 
   # time records.config is edited
   if ! grep "^CONFIG proxy.config.proxy_name STRING deflect_ats$" \
	${EDGEBASE}${TSPATH}/conf/records.config >/dev/null ; then
     RECORDS_TMP=$(mktemp)
     sed -e "s%^CONFIG proxy.config.proxy_name STRING.*%CONFIG proxy.config.proxy_name STRING deflect_ats%" \
	${EDGEBASE}${TSPATH}/conf/records.config > ${RECORDS_TMP}
     mv ${RECORDS_TMP} ${EDGEBASE}${TSPATH}/conf/records.config
     unset RECORDS_TMP
   fi
}

verbose() {
  echo DEBUG: Doing "$@"
  "$@"
}

do_clean_sync() {
  echo "clean synchronising files in $CSYNCPATH to $EDGENAME"
  [ -z "$CSYNCPATH" ] && echo "tried to sync but CSYNCPATH not set - exiting" && exit 1
  cd $EDGEBASE
  $V rsync -avpzRL --del -e "ssh" --exclude='old/' ./$CSYNCPATH root@${EDGENAME}:/
}

iptables_safe() {
  # the iptables.ddeflect script applies the new firewall rules then sleeps
  # after which time it check for the flag we set. if that
  # flag is still present - ie, we were not able to delete it - firewall
  # rules are flushed completely.
  echo -en "installing clean deny lists to $EDGENAME\n"
  CSYNCPATH=etc/ufw/deny.d
  do_clean_sync

# Write script to set danger flag and reload ufw
local SAFERELOAD=$(cat <<'_END'
#!/bin/dash
touch /etc/ufw/iptables.ddanger || ( echo FAILED TO SET FLAG quitting ; exit )
# Tried a reload here and it only worked 75% of the time.
/usr/sbin/ufw --force disable > /dev/null 2>&1
/usr/sbin/ufw --force enable > /dev/null 2>&1
COUNT=60
while [ "$COUNT" -ne "0" ]
do
        sleep 1
        COUNT=$(($COUNT-1))
        if [ ! -f /etc/ufw/iptables.ddanger ]; then
                break
        fi
done

DATE=$(date "+%Y/%m/%d %H:%M:%S")
if [ -f /etc/ufw/iptables.ddanger ] ; then
        /usr/sbin/ufw --force disable > /dev/null 2>&1 || echo "Failed to disable ufw"
	/sbin/iptables -F > /dev/null 2>&1
	/sbin/iptables -X > /dev/null 2>&1
	/sbin/iptables -t nat -F > /dev/null 2>&1
	/sbin/iptables -t nat -X > /dev/null 2>&1
	/sbin/iptables -t mangle -F > /dev/null 2>&1
	/sbin/iptables -t mangle -X > /dev/null 2>&1
	/sbin/iptables -P INPUT ACCEPT > /dev/null 2>&1
	/sbin/iptables -P FORWARD ACCEPT > /dev/null 2>&1
	/sbin/iptables -P OUTPUT ACCEPT > /dev/null 2>&1
	echo "\033[1mProblem: Something went wrong with firewall install. Fix before trying again.\033[0m"
	echo "\n\033[1mYou do not have a firewall on your edge.\033[0m"
        echo "$DATE autoflushing iptables" >> /var/log/iptables.dautoflush
  else
        echo "$DATE flag not present - update successful" >> /var/log/iptables.dautoflush
fi
_END
)

# Write the simple script over to edge to load deny list(s)
local DNYSCRIPT=$(cat <<'_END'
#!/bin/dash
# Note the she-bang is /bin/sh, not /bin/bash .. fixed so point to /bin/dash 

UFW=/usr/sbin/ufw
LOADDIR=/etc/ufw/deny.d
USERINPUT=/lib/ufw/user.rules

# Check id ufw is installed
if [ ! -x "${UFW}" ]; then
  echo
  echo "\033[1mProblem: ufw not installed. No firewall\033[0m"
  exit
fi

# First we get our 2 temp file names and hope they are in SHM 
# This is not fail-proof. If we can write to /dev/shm then this will succeed
MYFILE1=$(mktemp -q --suffix=_deflect_firewall --tmpdir=/dev/shm)
MYFILE2=$(mktemp -q --suffix=_deflect_firewall --tmpdir=/dev/shm)
if [ -w "${MYFILE1}" ] && [ -w "${MYFILE2}" ]; then
  echo -n "\nEXCELENT: Using TMPFS for output. "
else
  MYFILE1=$(mktemp -q --suffix=_deflect_firewall)
  MYFILE2=$(mktemp -q --suffix=_deflect_firewall)
  if [ -w "${MYFILE1}" ] && [ -w "${MYFILE2}" ]; then
    echo -n "\nWARNING: Not using TMPFS for output. May have slower results\nin building large block list. Mount your /dev/shm. "
  else
    echo "\nERROR: No writable file for output. Find why we can not create\nand write to a temp file\n"
    exit
  fi
fi
echo "Done."

# Set counter of IPs
k=0;
# Start building file that is going to be inserted into the rules into MYFILE1
echo -n "Starting the build of rules file. "
if [ -d $LOADDIR ] && [ -f $USERINPUT ]; then
  for f in ${LOADDIR}/*; do
    if [ -f $f ]; then
      for i in $(grep -v ^#); do
        echo "\n### tuple ### deny any any 0.0.0.0/0 any $i in\n-A ufw-user-input -s $i -j DROP"  
        k=$(($k+1)) 
      done < $f
    fi
  done
else
 echo "Failed." >> /dev/stderr
 exit
fi > $MYFILE1
echo "Done.                           "

# Copy our current file and insert new content, blocks, in.
cat $USERINPUT > $MYFILE2

# Now insert file 
echo -n "Bulding ${k} IPs into block file ${USERINPUT}. "
while read line; do
if [ "${line}" = "### RULES ###" ]; then
  echo $line
  cat $MYFILE1
  rm $MYFILE1
else
  echo $line
fi
done < $USERINPUT > $MYFILE2
echo "Done."

# Over write our original file
echo -n "Installing rules to ${USERINPUT}. "
cat $MYFILE2 > $USERINPUT
rm $MYFILE2
echo "Done."
_END
)
  echo -en "Bulding deny list on $EDGENAME\n\n"
  $V ssh -l root $EDGENAME "echo '$DNYSCRIPT' > /etc/ufw/dn.sh && chmod +x /etc/ufw/dn.sh && /etc/ufw/dn.sh"
  echo "applying firewall and setting danger flag to avoid lockout"
  $V ssh -f -l root $EDGENAME "( echo '$SAFERELOAD' > /etc/ufw/iptables.ddeflect && chmod +x /etc/ufw/iptables.ddeflect && /etc/ufw/iptables.ddeflect & ) "
  echo -ne 'waiting: #                                     (1%)\r' && sleep 1
  echo -ne 'waiting: #######                              (25%)\r' && sleep 1
  echo -ne 'waiting: ###################                  (50%)\r' && sleep 1
  echo -ne 'waiting: ############################         (75%)\r' && sleep 1
  echo -ne 'waiting: ################################### (100%)\r' && sleep 1
  echo
  echo "removing danger flag"
  echo "if flag not removed in approx 1 minute the firewall will flush and not be set" 
  $V ssh -l root $EDGENAME "( rm -fr /etc/ufw/iptables.ddanger ) "
}

do_edge_check() {
  [ -n "$USE_RELOAD" ] && echo "sleeping 1 before edge_check ..." && sleep 1 || (echo -n "sleeping 3 before edge_check" ; echo -n " ." ; sleep 1 ; echo -n "." ; sleep 1 ; echo -n "." ; sleep 1 ; echo "")
  # See if we are using a alternate config 
  if [ -z "ALT" ]
  then
    edge_check.sh -H $EDGENAME -q -c 2 -t
  else
    edge_check.sh -A $alt_conf -H $EDGENAME -q -c 2 -t
  fi
}

do_build() {
  ### ssh to the host and do initial config
  AUTHKEY="`cat $ROOTPUBKEY`"
  TSAUTHKEY="`cat $TSPUBKEY`"
  echo -n "WARNING: build process includes a reboot, continue y/n? "
  read CONT
  [ "$CONT" != "yes" ] && [ "$CONT" != "y" ] && echo "response not yes or y - exiting" && exit

  ## Lets get key in asap so do not have to keep typing password
  $V ssh -l root $EDGENAME "\
  [ ! -d ~root/.ssh ] && mkdir ~root/.ssh; \
  echo "$AUTHKEY" >> ~root/.ssh/authorized_keys"

  if [ -n "$REPOSTRING_deflect" ]; then
      $V ssh -l root $EDGENAME "echo $REPOSTRING_deflect > /etc/apt/sources.list.d/deflect.list"
  fi
  if [ -n "$REPOSTRING_rsyslog" ]; then
      $V ssh -l root $EDGENAME "/bin/echo -e \"$REPOSTRING_rsyslog\" > /etc/apt/sources.list.d/rsyslog.list; \
      apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 894ECF17AEF0CF8E"
  fi


  $V ssh -l root $EDGENAME "apt-get update; \
	grep -ri '^[[:alpha:]].*CDROM' /etc/apt/sources* && echo "Remove CDROM lines from /etc/apt/sources.list then retry" && exit 1 ; \
	apt-get upgrade ; \
	export DEBIANTFRONTEND=noninteractive ; \
	for pkg in $TSPREREQS $NAGIOS $FAIL2BAN $MISC ; \
	  do apt-get -y --force-yes install \$pkg ; \
	done ; \
	/etc/init.d/apache* stop ; \
	apt-get -y remove apache2 ; \
        pip install swabber ; \
	/etc/init.d/nagios-nrpe-server restart"

  ### set up ENCfs if required
  if [ $ENCFS -eq 1 ]; then 
	ssh $EDGENAME mkdir -p /usr/local/trafficserver /usr/local/.trafficserver
	/usr/local/deflect/scripts/initENCfs.e $EDGENAME $ENCFS_PASS /usr/local/.trafficserver/ /usr/local/trafficserver/
	sleep 2
  fi

  $V ssh -l root $EDGENAME "useradd -u $TSUID -U -s /bin/bash -d $TSPATH -m $TSUSER ; \
	groupmod -g $TSGID $TSGROUP ; \
        [ ! -d $TSPATH/.ssh ] && mkdir $TSPATH/.ssh ; \
	echo "$TSAUTHKEY" >> $TSPATH/.ssh/authorized_keys ; \
	chown -R $TSUSER:$TSGROUP $TSPATH/.ssh ; \
	ldconfig ; \
	echo "$EDGENAME" > /etc/hostname; \
	hostname $EDGENAME"

  ### synchronise edge/ tree to host
  ats_hostname
  SYNCPATH="*"
  do_sync

  # run our iptables script
  iptables_safe

  # enable swabber on boot
  $V ssh -l root $EDGENAME "update-rc.d swabberd defaults"

  # We need to sleep here because to make sure above finished before we reboot.
  # make trafficserver start on boot, then reboot
  $V ssh -l root $EDGENAME "chmod 775 /etc/init.d/trafficserver ; update-rc.d trafficserver defaults ; init 6"

  ### wrap it up
  echo "$EDGENAME rebooted - sleeping 60s before final check"
  sleep 60
  # See if using a alternate config 
  if [ -z "$ALT" ]
  then
     $V edge_check.sh -H $EDGENAME -q -c 2
  else
     $V edge_check.sh -A $alt_conf -H $EDGENAME -q -c 2
  fi
  /bin/echo -e "\nDONE. You should now add $EDGENAME to monitoring\n\n"
  ats_unhostname
}

check_dnet () {
  for j in $(echo $HOSTS) ; do
    for i in $(egrep -v ^# $EDGES) ; do
      if [ "$j" = "$i" ]; then
        return 0;
      fi
    done
  done
return 1
}

##### invocation options (main)
# unset a few things we use for late (just in case)
unset BUILD EDGENAME FIREWALL NOOP NOSYNC RESTART SET_HOSTNAME SYNC SYNCONLY SYNCPATH V FAILED_EDGES REMOTE_CMD_FORCE

# check that we have been given some arguments, exit if not
[ -z "$1" ] && echo "More instructions required! Use '-h' for help" && exit 1

# Load our common func / vars and use alternate config 
source /usr/local/deflect/scripts/common.sh
source /usr/local/deflect/scripts/encfs_pass

# log the invocation before we go through all getops
echo `date +%c` $USER : $MYNAME $@ >>$DPLOGFILE

while getopts "A:blF:hH:inNp:P:sS:v" options ; do
case $options in
  b )
    BUILD=0
  ;;
  F )
    [ "$OPTARG" == "remotecmd" ] && REMOTE_CMD_FORCE=0
  ;;
  h )
    help ; exit
  ;;
  l )
    USE_RELOAD=0
  ;;
  H )
    case $OPTARG in
      ALL )
        HOSTS="`grep -v ^# $EDGES`"
      ;;
      LIVE )
        HOSTS="`grep -v ^# $EDGES_LIVE`"
      ;;
      NONLIVE )
        HOSTS="`for i in $(egrep -v ^# $EDGES) ; do grep ^$i $EDGES_LIVE >/dev/null 2>&1 || echo $i ; done`"
      ;;
      * )
        host ${OPTARG}.${DHOST} && HOSTS=${OPTARG}.${DHOST} \
       || ( echo $OPTARG is not a .${DHOST} host ; exit 1 )
	check_dnet || (echo -n "DANGER: $HOSTS not in this dnet. 'y' to continue: " ;\
        read CONT ; \
        [ "$CONT" = y ] || (echo " $CONT not 'y' - exiting"; exit 1)) || exit 
      ;;
    esac
  ;;
  i )
    IGNORE_PING=0
  ;;
  n )
    echo "No-op mode"
    NOOP=0
  ;;
  N )
    echo "No-sync mode- files will not be copied before service is restarted"
    NOSYNC=0
  ;;
  p )
    case $OPTARG in
      ALL )
	SYNCPATH="*"
	RESTART_CMD="/etc/init.d/trafficserver stop ; service nagios-nrpe-server restart ; service log-courier restart ; service trafficserver start ; service swabberd restart"
	RRESTART_CMD="/etc/init.d/trafficserver reload ; service nagios-nrpe-server restart ; service log-courier restart ; service swabberd restart"
	echo "-p ALL will sync all files under $EDGEBASE and restart all except iptables script"
	SET_HOSTNAME=0
	CHECK=0
      ;;
      ats|atsconf )
	SYNCPATH="usr/local/trafficserver/conf"
	RESTART_CMD="touch /usr/local/trafficserver/conf/remap.d ; /etc/init.d/trafficserver restart"
	RRESTART_CMD="touch /usr/local/trafficserver/conf/remap.d ; /etc/init.d/trafficserver reload"
	SET_HOSTNAME=0
	CHECK=0
      ;;
      atsmods )
	SYNCPATH="usr/local/trafficserver/modules"
	RESTART_CMD="touch /usr/local/trafficserver/conf/remap.d ; /etc/init.d/trafficserver restart"
	RRESTART_CMD="touch /usr/local/trafficserver/conf/remap.d ; /etc/init.d/trafficserver reload"
	SET_HOSTNAME=0
	CHECK=0
      ;;
      atsall )
	SYNCPATH="usr/local/trafficserver"
	RESTART_CMD="touch /usr/local/trafficserver/conf/remap.d ; /etc/init.d/trafficserver restart"
	RRESTART_CMD="touch /usr/local/trafficserver/conf/remap.d ; /etc/init.d/trafficserver reload"
	SET_HOSTNAME=0
	CHECK=0
      ;;
      fail2ban )
	SYNCPATH="etc/fail2ban"
	RESTART_CMD="/etc/init.d/fail2ban restart"
      ;;
      firewall )
	SYNCPATH="etc/ufw lib/ufw etc/default/ufw"
	FIREWALL=0
	CHECK=0
	RESTART_CMD="/etc/init.d/swabberd restart"
      ;;
      rsyslog )
	SYNCPATH="etc/rsyslog.d etc/apt/sources.list.d/rsyslog.list"
	RESTART_CMD="service rsyslog restart"
      ;;
      misc )
	SYNCPATH=$MISC_PATHS
	RESTART_CMD=""
      ;;
      swabber )
        SYNCPATH="etc/swabber.yaml etc/init.d/swabberd"
	RESTART_CMD="service swabberd restart"
      ;;
      conftouch )
        SYNCPATH="usr/local/trafficserver/conf/dashboard.timestamp"
	TOUCHONLY=0
        HOSTS="`grep -v ^# $EDGES`"
      ;;
      nagios )
	SYNCPATH="etc/nagios usr/local/nrpe"
	RESTART_CMD="/etc/init.d/nagios-nrpe-server restart"
      ;;
      logcourier )
	SYNCPATH="etc/log-courier"
	RESTART_CMD="service log-courier restart"
      ;;
      * )
	echo "package $OPTARG not valid - use '-h' for instructions" && exit 1
      ;;
    esac
  ;;
  P )
    REMOTE_CMD="$OPTARG"
    echo "Will additionally run $REMOTE_CMD"
    CHECK=0
  ;;
  s )
    SYNCONLY=0
  ;;
  S )
    SYNCPATHX="$OPTARG"
  ;;
  v )
    V=verbose
  ;;
  esac
done

# if NOOP is set change verbose->echo
[ -n "$NOOP" ] && V="echo NOOP, would have done: "

# without HOSTS, exit
[ -z "$HOSTS" ] && echo "Specify a host -- use '-h' for help" && exit 1

# make sure -S works in case someone gives a -p too
[ -n "$SYNCPATHX" ] && SYNCPATH="$SYNCPATHX"

# If ats needs set to use reload
if [ -n "$USE_RELOAD" ] ; then
  [ -n "$RRESTART_CMD" ] && RESTART_CMD="$RRESTART_CMD"
fi

# merge remote commands
if [ -n "$REMOTE_CMD" ] ; then
  [ -n "$RESTART_CMD" ] && RESTART_CMD="$RESTART_CMD ; $REMOTE_CMD" || RESTART_CMD="$REMOTE_CMD"
  if [ -z "$TOUCHONLY" ] ; then
    echo -e " Full remote command is now \"${RESTART_CMD}\""
    if [ -z "$REMOTE_CMD_FORCE" ] ; then
      echo " DANGER: do you really want to run this? 'y' to continue: "
      read CONT
      [ "$CONT" = y ] || ( echo "response $CONT not 'y' - exiting" && exit 1 )
     else
      echo " FORCING EXECUTION OF $REMOTE_CMD"
    fi
  fi
fi

DNETLOCK=/var/run/$MYNAME.$alt_conf.lock
if [ -f $DNETLOCK ]; then
  echo
  echo "$MYNAME is already running for DNET $alt_conf" 
  echo "if you are sure this script is not running you can"
  echo "remove file $DNETLOCK"
  echo
  exit 6
fi
# trap on exit
trap finish EXIT
touch $DNETLOCK

# email a report about the invocation
[ -n "$SUDO_USER" ] && IAM=$SUDO_USER || IAM=$USER
#echo -e "\nWhat: $MYNAME $@\nWhen: `date`\nWho: $IAM\n" | mailx -s "deploy.sh invoked" $MAILRCPT

##### execution

### pre
# set up an ssh-agent if we don't already have one, and kill it after if we created
# it now
[ -z "$SSH_AUTH_SOCK" ] && eval `ssh-agent` && OUR_AGENT=$SSH_AGENT_PID
ssh-add -ls 2>&1| grep `ssh-keygen -lf $ROOTPRIVKEY | awk '{print $2}'` >/dev/null || ssh-add $ROOTPRIVKEY
ssh-add $TSPRIVKEY

### main
for EDGENAME in $HOSTS ; do
  echo "!!! BEGINNING $EDGENAME NOW !!!"
  if ping -c1 -w2 $EDGENAME >/dev/null 2>&1 || [ "$IGNORE_PING" = "0" ] ; then
    if [ "$BUILD" = 0 ] ; then
      grep ^$EDGENAME $EDGES_LIVE && echo "found edge in live list - remove before build" && exit 1
      echo "Building for $HOSTS"
      do_build
      exit
     else
	# check that the encfs.deflect flag is present before deploying.
	# if it's not present, try once to mount encfs then check again
      if ! ssh $EDGENAME "head -1 /usr/local/trafficserver/encfs.deflect 2>/dev/null" | grep "^TSPxau2fgvtmMVmstZHIjmOZErX5FGIlBRE2EO99DcWsdJNQi5IPRiNFiRAUX0ffghcSxeiFSfLhLdNLPVakvF21UpBVt8hjck9L$" >/dev/null ; then
         mountats2 ${EDGENAME%.$DHOST}
         if ! ssh $EDGENAME "head -1 /usr/local/trafficserver/encfs.deflect 2>/dev/null" | grep "^TSPxau2fgvtmMVmstZHIjmOZErX5FGIlBRE2EO99DcWsdJNQi5IPRiNFiRAUX0ffghcSxeiFSfLhLdNLPVakvF21UpBVt8hjck9L$" >/dev/null ; then
         echo -e "\n\n>>>>>>> DEFLECT ENCFS NOT DETECTED - INVESTIGATE!"
	 echo "Tried to mountats, but doesn't seem to have worked"
         echo -e ">>>>>>> SKIPPING HOST $EDGENAME\n\npress return to continue deploying to remaining edges"
         ENCFS_FOUND=FALSE
         read
          else
           ENCFS_FOUND=TRUE
          fi
         else
          ENCFS_FOUND=TRUE
        fi
      if [ "$ENCFS_FOUND" == TRUE ] ; then
        # if we have no syncpath, and no remote command, then warn and exit
        [ -z "$SYNCPATH" ] && [ -z "$REMOTE_CMD" ] && echo "need more arguments - see '-h'" && exit 1
        [ -n "$SET_HOSTNAME" ] && ats_hostname
        [ -z "$NOSYNC" ] && [ -n "$SYNCPATH" ] && do_sync
        if [ -z "$SYNCONLY" ] ; then
    	    [ "$FIREWALL" = 0 ] && echo "Firewall push also restarts (but does not update) Swabberd" && iptables_safe
	    [ -n "$RESTART_CMD" ] && do_restart
        else
	    echo "Sync only mode - nothing else to do for ${EDGENAME}"
        fi
        [ "$CHECK" = 0 ] && do_edge_check
        [ -n "$SET_HOSTNAME" ] && ats_unhostname
      fi
      echo "!!! END $EDGENAME NOW !!!"
    fi
  else
    echo "<<<< SKIPPED $EDGENAME DUE TO FAILED PING >>>>"
    FAILED_EDGES="$FAILED_EDGES $EDGENAME"
  fi
done

### post

[ -n "$FAILED_EDGES" ] && echo "Some edges failed to respond to a ping and were therefore not updated:$FAILED_EDGES"

# kill our ssh-agent - but only if we created it
[ -n "$OUR_AGENT" ] && kill $OUR_AGENT && echo "killing ssh-agent (${OUR_AGENT})"
exit

##### END
