#!/bin/bash

# renew only the named certs in LE, when they are about to expire
# and place all certs where they can be easily synced to brains
#
# DOMAINS_DIR should contain one file per certificate, containing
# all the SNI names the certificate should contain in the format:
#   maindomain.com -d sub.maindomain.com -d sub.maindomain.com
#
#

# define some variables
LE_DOMAINS_LIST=/opt/autodeflect/etc/letsencrypt-site.list
LE_OUTPUT_DIR=/opt/autodeflect/le_certs
RENEW_WINDOW=29days   # `man date` for format details
RENEW_DATE=$(date -d "+$RENEW_WINDOW" +%s)

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
  certbot certonly --standalone --agree-tos --standalone-supported-challenges http-01 -m sysops@equalit.ie -n -d $DOMAIN_W_ARGS
}

function populate_output_dir() {
  LE_PREFIX=/etc/letsencrypt/live/${DOMAIN_W_ARGS%% *}
  cp ${LE_PREFIX}/cert.pem ${LE_OUTPUT_DIR}/${DOMAIN_W_ARGS%% *}.cert.crt
  cp ${LE_PREFIX}/chain.pem ${LE_OUTPUT_DIR}/${DOMAIN_W_ARGS%% *}.chain.crt
  cp ${LE_PREFIX}/privkey.pem ${LE_OUTPUT_DIR}/${DOMAIN_W_ARGS%% *}.key
}

function initialise_output_dir(){
  [ -d ${LE_OUTPUT_DIR} ] && rm -r ${LE_OUTPUT_DIR}/* || mkdir -p ${LE_OUTPUT_DIR}
}

# This is a bit hard to read, so explained in detail:
# - create an empty directory for cert bundles to end up in
# - grep out any lines beginning with a '#" character or empty
# - pipe that output to a while loop that reads line by line
# - run the "check_cert()" function defined above to see whether
#   we already have an up to date version of this cert
# - if we do not, go ahead and request a cert (including any sub-
#   domains that should be included)
# - place all the cert/key bundles into an easy dir to be synced
#   to brains
initialise_output_dir
egrep -v '^(#|$)' $LE_DOMAINS_LIST | while IFS='' read -r DOMAIN_W_ARGS ; do
  check_cert || renew_cert 
  populate_output_dir
done
