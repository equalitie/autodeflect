#!/bin/bash

LIVEDIR=/usr/local/deflect/ansible/autobrains

USER=$(id -nu)
PWD=$(pwd)

if [ "$USER" == "root" ]; then
  echo "Do not run this as root"
  exit
fi

if [ "$PWD" == "$LIVEDIR" ]; then
  echo "Can not run this is same directory as LIVEDIR"
  exit
fi

if [ ! -d "roles" ]; then
  echo "This script should be ran in ansible playbook_dir"
  exit
fi 

# site.yml
# hosts.yml
# config/
# override.yml
# .aw_lastrun
# clients.yml
# clients.yml-revisions/

# Exit on error: if anything fails to copy from the LIVEDIR, it is not worth continuing
set -e

# This file just needs to exist. autobrains_update.sh should link correctly
rm -f clients.yml
touch clients.yml

# First copy files
rsync $LIVEDIR/site.yml site.yml
rsync $LIVEDIR/hosts.yml hosts.yml
rsync $LIVEDIR/override.yml override.yml
rsync $LIVEDIR/.aw_lastrun .aw_lastrun

rsync -a $LIVEDIR/config/ config
rsync -a $LIVEDIR/clients.yml-revisions/ clients.yml-revisions

ansible-playbook site.yml -l controller --tags init

echo "On first run, you should run autobrain_update.sh and answer y to update clients.yml"
