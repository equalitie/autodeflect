#!/bin/bash

# renew only the named certs in LE, when they are about to expire
# and place all certs where they can be easily synced to brains
#
#

# define some variables
BASEDIR=/opt/autodeflect/
LE_SITE_LIST=${BASEDIR}/etc/letsencrypt-site.list
LE_OUTPUT_DIR=${BASEDIR}/le_certs
RENEW_WINDOW=29days   # `man date` for format details
RENEW_DATE=$(date -d "+$RENEW_WINDOW" +%s)
LE_CERTS_CONFIG=${BASEDIR}/etc/letsencrypt-certs.auto
LE_DNETS_CONFIG=${BASEDIR}/etc/letsencrypt-dnets.auto

# split config into relevant sections
grep '^CERTS' $LE_SITE_LIST | cut -d" " -f 2- > $LE_CERTS_CONFIG
grep '^DNET' $LE_SITE_LIST | cut -d" " -f 2- > $LE_DNETS_CONFIG

function check_cert() {
  if [ -f /etc/letsencrypt/live/${DOMAIN_W_ARGS%% *}/cert.pem ] ; then
    # found a cert - check it's expiry
    EXPIRY=$(date -d "$(openssl x509 -noout -dates -in /etc/letsencrypt/live/${DOMAIN_W_ARGS%% *}/cert.pem | awk -F'=' '/After/ {print $2}')" +%s)
    if [ $EXPIRY -gt $RENEW_DATE ]; then
      # not expiring within RENEW_WINDOW
      STATUS=0
     else
      # expiring within RENEW_WINDOW
      STATUS=1
    fi
   else
     # cert is missing completely
     STATUS=2
  fi
return $STATUS
}

function renew_cert() {
  echo certbot certonly --standalone --agree-tos --standalone-supported-challenges http-01 -m sysops@equalit.ie -n -d $DOMAIN_W_ARGS
}

function populate_output_dir() {
  LE_PREFIX=/etc/letsencrypt/live/${SITEDNET%%:*}
  cp ${LE_PREFIX}/cert.pem ${LE_OUTPUT_DIR}/${SITEDNET##*:}/${SITEDNET%%:*}.cert.crt
  cp ${LE_PREFIX}/chain.pem ${LE_OUTPUT_DIR}/${SITEDNET##*:}/${SITEDNET%%:*}.chain.crt
  cp ${LE_PREFIX}/privkey.pem ${LE_OUTPUT_DIR}/${SITEDNET##*:}/${SITEDNET%%:*}.key
}

# This is a bit hard to read, so explained in detail:
# - grep out any lines beginning with a '#" character or empty
# - pipe that output to a while loop that reads line by line
# - run the "check_cert()" function defined above to see whether
#   we already have an up to date version of this cert
# - if we do not, go ahead and request a cert (including any sub-
#   domains that should be included)
# - place all the cert/key bundles into an easy dir to be synced
#   to brains
cat $LE_CERTS_CONFIG | while IFS='' read -r DOMAIN_W_ARGS ; do
  check_cert || renew_cert 
done

initialise_output_dir

cat $LE_DNETS_CONFIG | while IFS='' read -r SITEDNET ; do
  populate_output_dir
done
