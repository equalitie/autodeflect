## Synopsis

This project consists of progams to automatically detect when client has updated the Dashboard configuration and then run the scripts needed to update the Deflect network.

* autodeflect "The control program used to start and stop all programs"
* ad2ssh <sshkey> "Starts ssh-agent and loads ssh-key to be shared between all programs"
* ad2p <process> "When triggered this program runs any update scripts needed and removes the process trigger file"
* ad2runner <runner> "Checks if update needs to happen and writes trigger file for <process>"

## Installation

Depends on:
* libssh2
* openssl

Debian/Ubuntu (7.x/14.04)
* apt-get install libssh2-1-dev libssl-dev

After ansible has ran tag init 'ansible-playbook site.yml -l controller --tags init' 
* cd daemon/
* cp cglobals.h.sample cglobals.h
* make install
* cp daemon.cfg.example ../bin/daemon.cfg
* cd ../bin
* edit daemon.cfg
* check daemon.cfg with 'autodeflect --config daemon.cfg --show-conf'
* (optional) if trigger.sh exists and is executable in directory_script it will run and if a exit status of 200,
  the process process file will trigger a run. This is meant to easily run external programs and exit a 200 if
  need to run the update process. 

## Usage 

### daemon programs and 'autodeflect' control program lives in bin/ directory.

* autodeflect with no options
* Typical start and stop
  * autodeflect --config daemon.cfg --all --start
  * autodeflect --config daemon.cfg --all --stop
* start or stop a single program
  * autodeflect --config daemon.cfg --sshkey --start
  * autodeflect --config daemon.cfg --sshkey --stop

## Tests

You can run any of the program with the --debug flag and program will print useful information to screen while running.

## Installing updates in production

For example, to make a hard-coded config change, ie changing config.c, and get it live...

Locally:
* git clone/pull
* vi daemon/config.c
* git add daemon/config.c ; git commit -m "YOURMESSAGE"
* git push

Then in production:
* cd /path/to/autodeflect
* git pull
* cd daemon
* make ; make install
* cd ../bin
* ./autodeflect --config daemon.cfg --all --stop
* ./autodeflect --config daemon.cfg --all --start

## Contributors

* "Rodney Mosley (RamJett)" 'rodney at equalit dot ie'
* "Anonymous" 'sparklinux'

* See: https://wiki.deflect.ca
* Special contribution https://ramnic.com

