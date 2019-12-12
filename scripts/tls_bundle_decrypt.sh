#!/bin/bash

# invocation:
#  $(basename) <sitename> <timestamp> <tls_home> <dnet> <gpg_home>

SITE=$1
BUNDLE=$2
TLS_HOME=$3
OUTPUT_DIR=${TLS_HOME}/output/$4
TLS_CERT_NAME=${SITE}-${BUNDLE}
GPG_HOME=$5

# List of TLDs taken from https://publicsuffix.org/list/effective_tld_names.dat
TLD_LIST="$(dirname $0)/effective_tld_names.dat"
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

# check that the decrypted cert and key match, and that the cert contains
# sensible SNI names only. if so, install into dnet output dir. otherwise
# error and exit.
function openssl_md5 {
    openssl $1 -noout -modulus -in $2 | openssl md5
}

function sni_names {
    openssl x509 -in $TLS_HOME/decrypted/$TLS_CERT_NAME.cert.crt -text -noout |\
      egrep 'DNS:|Subject:' |\
      sed 's/\ //g' |\
      sed -e 's%.*CN=%%' -e 's% *DNS:%%g' |\
      tr ',' '\n'
}

function sni_wildcard {
    WILDCARD=false
    SITETMP=${SITE#*.}
    until grep $SITETMP "$TLD_LIST" > /dev/null ; do
        if sni_names | grep "\*\.$SITETMP" >/dev/null ; then
            WILDCARD=true
            break
        fi
        SITETMP=${SITETMP#*.}
    done
    if [ "$WILDCARD" == true ] ; then
        true
    else
        false
    fi
}

md5sum_cert=$(openssl_md5 x509 $TLS_HOME/decrypted/$TLS_CERT_NAME.cert.crt)
md5sum_key=$(openssl_md5 rsa $TLS_HOME/decrypted/$TLS_CERT_NAME.key)

if [ "$md5sum_cert" == "$md5sum_key" ] ; then
    if sni_names | grep -v "^.*${SITE}$" >/dev/null; then
        if ! sni_wildcard ; then
            echo "names found do not match $SITE - not allowing. Found:"
            sni_names
            exit 2
        fi
    fi
    cp $TLS_HOME/decrypted/$TLS_CERT_NAME.* $OUTPUT_DIR
  else
    echo "key and certificate for $TLS_CERT_NAME do not match - not installing"
    exit 2
fi
