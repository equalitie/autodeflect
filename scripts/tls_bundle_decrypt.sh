#!/bin/bash

# invocation:
#  $(basename) <sitename> <timestamp> <tls_home> <dnet> <gpg_home>

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
  for FILETYPE in $FILETYPES ; do
    gpg --homedir $GPG_HOME -d --output $TLS_HOME/decrypted/${TLS_CERT_NAME}.$FILETYPE $TLS_HOME/encrypted/${TLS_CERT_NAME}.${FILETYPE}.gpg
  done
fi

# check that the decrypted cert and key match!
# if so, install into dnet output dir. otherwise error
openssl_md5() { openssl $1 -noout -modulus -in $2 | openssl md5 ; }
md5sum_cert=$(openssl_md5 x509 $TLS_HOME/decrypted/TLS_CERT_NAME.cert.crt)
md5sum_key=$(openssl_md5 rsa $TLS_HOME/decrypted/TLS_CERT_NAME.key)
if [ "$md5sum_cert" == "$md5sum_key" ] ; then
    cp $TLS_HOME/decrypted/$TLS_CERT_NAME.* $OUTPUT_DIR
  else
    echo "key and certificate for $TLS_CERT_NAME do not match - panic!"
    exit 2
fi
