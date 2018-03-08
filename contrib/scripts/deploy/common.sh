#!/bin/bash

# The config file must have default values first so if a alternate configuration is defined,
# the alternate configuration will over write the variable.

# This file is a common file for our varables and INI style custom varable over-rides 
DEFLECT_CONF=/usr/local/deflect/etc/deflect.config 

# Make sure our scripts using this file will look in our scripts directory first
PATH=/usr/local/deflect/scripts:$PATH

# make sure we are able to use getops to get -A 
unset options
unset OPTIND
ALT=0

# Just get our -A here globally
# if we pass (-A default) we will just display all avaible
while getopts ":A:" options ; do
case $options in
 A )
    ALT=1
    alt_conf="$OPTARG"
 ;;
  esac
done

# make sure we set it back so our scripts can use getops
unset options
unset OPTIND

# Set our counter to 0
CCC=0

while IFS='= ' read var val
do
    # Skip commented lines. Line that begin with # or ;
    if [[ $var =~ ^# || $var =~ ^\; ]]
    then
	continue
    fi
    # Test is $var is a INI file block definition. 
    if [[ $var = \[*\] ]]
    then
       	section=$var
	arr_alt[$CCC]=$var
	let CCC=CCC+1	
    elif [[ $val ]]
    then
	if [[ $var$section = $var\[default\] ]]
	then
		eval "$var=${val}"
	elif [[ ($var$section = $var\[$alt_conf\] && $alt_conf != "") ]]
	then
		alt_found=1
		eval "$var=${val}"
	else
		continue
	fi

    fi
done < $DEFLECT_CONF

if [[ ( "$ALT" -eq "1" && "$alt_found" -ne "1" )  || "$alt_conf" = "default" || ( "$FORCE_ALT" -eq "1" && "$ALT" -ne "1" ) ]]
then
	SHOWALT=1
fi

if [[ "$SHOWALT" -eq "1" ]] ; then
 echo -e "\nListing all available alternate configurations in deflect.config file\nbecause you requested a list or your command was not correct\n"

 if [[ "$alt_conf" = "default" ]]
 then
	echo -e "You selected [default]. This can not be selected with the -A option\n"
 elif [[ "$FORCE_ALT" -eq "1" && "$ALT" -ne "1" ]]
 then
	echo -e "You have FORCE_ALT configured in deflect.config and you did not use -A\n"
 else
	echo -e "You requested -A "$alt_conf" option and it was not found.\n"
 fi
 echo "Pick from the list below. [default] can not be used."
 echo "===================================" 
  for item in ${arr_alt[*]}
  do
    printf "   %s\n" $item
  done
 echo "==================================="
exit
fi
