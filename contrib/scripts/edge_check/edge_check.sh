#!/bin/bash
#
#
# request a URL from all edges
# 

# Defaults for this file
QUIET=1
LIVE=LIVE
TIMEOUT=9
COUNT=1
TLS=1
UA="edge_check.sh"

# Use common.sh for common func / varables
source /usr/local/deflect/scripts/common.sh

while getopts "A:c:thH:l:m:nqS:u:w:o:p:U:" options ; do
case $options in
c )
COUNT=$OPTARG
;;
h )
echo "Send simple HTTP requests to all live or non-live edge servers
  Usage:
  -c [ count ]		repeat requests to each edge N times
  -h			print this help
  -H [ host/IP ]	target a single server
  -l [ filepath ]	use an alternative target server list
  -m [ METHOD ]		HTTP method, supports HEAD and GET (default: GET)
  			nb: HEAD requests do not result in cache-store
  -n			target non-live edge caches instead of live caches
  -q			be quiet - less output
  -t			make a https request
  -p [ port ]		specify post (default: 80) -t will (default: 443)
  -S [ Host header ]	Send custom Host: header (default: $DHOST)
     [ ALL ]		parse remap.config for TEST= comments and iterate through all
  			Host headers for our live or near-live partners
  -u [ URI ]		object path (default: '/')
  -U [ User-agent ]	Set an alternate User-agent header
  -w [ timeout ]	timeout in seconds for nc (default: 4) 
  -A [ alt_conf ]       Use alternate configuration variables from deflect.config
                        Do not use -A to use [default] . -A default will return
                        all found alt config names in config file.
   -o [ live edge file] Use a alternate edge file -o file_path
"
exit
;;
S )
if [ "$OPTARG" = "ALL" ]; then
  HOSTLIST=$(grep ^\.include /uld/edges/$alt_conf/usr/local/trafficserver/conf/remap.config  | sed -e 's%.*/%%' -e 's%.config$%%')
  HOSTS=$(echo $HOSTLIST)
  echo -e "Host: header list from remap.config expands to \n$HOSTS\n\nUsing '-S all' produces a lot of output- press return to continue..." ; read
 else
  HOSTS=$OPTARG
fi
;;
m )
METHOD=$OPTARG
;;
p )
PORT=$OPTARG
;;
n )
LIVE="NON-LIVE"
;;
t )
TLS=0
;;
o )
EDGES_LIVE=$OPTARG
echo using $EDGES_LIVE as host list
;;
H )
TGT_HOST=$OPTARG
;;
q )
QUIET=0
;;
u )
URI=$OPTARG
;;
U )
UA=$OPTARG
;;
w )
TIMEOUT=$OPTARG
;;
esac ; done

# define the targets
if [ -n "$TGT_HOST" ] ; then
	REDGES=$TGT_HOST
  elif [ "$LIVE" = "LIVE" ] ; then
	REDGES="`egrep -v '^#' $EDGES_LIVE | awk '{print $1}'`"
  else
	REDGES=$(for i in $(egrep -v ^# $EDGES) ; do grep ^$i $EDGES_LIVE >/dev/null 2>&1 || echo $i ; done)
fi

[ -z "$HOSTS" ] && HOSTS="$DHOST"
[ -z "$URI" ] && URI='/'
[ -z "$METHOD" ] && METHOD=GET
if [ -z "$PORT" ] ; then
  if [ $TLS = 0 ] ; then
	PORT=443
  else
	PORT=80
  fi
fi

if [ $TLS = 0 ] ; then
  https="--ssl"
else
  https=""
fi

echo -e "checking with HOSTS=$HOSTS and URI=$URI $COUNT times per edge server
edge list contains:
`echo $REDGES`
"

do_request() { echo -e "$METHOD $URI HTTP/1.1\nHost: ${1}\nUser-Agent: ${UA}\nConnection: close\n\n" ; }

for edge in $REDGES ; do
	for HOST in $HOSTS ; do
	  REQS=0
	  echo ----- BEGIN ${HOST}${URI} @ $edge -----
	  until [ $REQS = $COUNT ] ; do
	    if [ $QUIET = 0 ] ; then
	      do_request $HOST | /usr/bin/time -f "time taken: %e" /usr/bin/ncat ${https} -w $TIMEOUT $edge $PORT 2>&1 | egrep '^(HTTP|BEGIN|Via|Location)|time taken|refused$'
	     else
	      do_request $HOST | /usr/bin/time -f "time taken: %e" /usr/bin/ncat ${https} -w $TIMEOUT $edge $PORT 2>&1
	    fi
	  REQS=$(($REQS + 1))
	  done
	done
done

