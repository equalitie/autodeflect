#!/bin/bash

# tls_bundle_decrypt.sh letstest.equalit.ie 1485240708 /tmp/tomr deflect1 /uld/ansible/autobrains/config/gpg

SITE=$1
BUNDLE=$2
TLS_HOME=$3
OUTPUT_DIR=${TLS_HOME}/output/$4
TLS_CERT_NAME=${SITE}-${BUNDLE}
GPG_HOME=$5

FILETYPES="cert.crt key chain.crt"

# ensure our dirs exist
[ -d ${TLS_HOME}/encrypted ] || mkdir ${TLS_HOME}/encrypted
[ -d ${TLS_HOME}/decrypted ] || mkdir ${TLS_HOME}/decrypted

# ensure that we have a decrypted copy
if ! [ -f $TLS_HOME/decrypted/$TLS_CERT_NAME.cert.crt ] ; then
  # if not, check that we have an encrypted version, or download
  #if ! [ -f $TLS_HOME/encrypted/$TLS_CERT_NAME.cert.crt ] ; then
      # or possibly just rsync them all over
  #fi
  for FILETYPE in $FILETYPES ; do
    gpg --homedir $GPG_HOME -d --output $TLS_HOME/decrypted/${TLS_CERT_NAME}.$FILETYPE $TLS_HOME/encrypted/${TLS_CERT_NAME}.${FILETYPE}.gpg
  done
fi

# install the decrypted copy into the right dnet dir
cp $TLS_HOME/decrypted/$TLS_CERT_NAME.* $OUTPUT_DIR
