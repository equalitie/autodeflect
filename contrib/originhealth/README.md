## Installation

Requires:
* php-5.4+
* composer
* autodeflect
* edgemanage (edge lists)
* phantomjs (composer will pull in to copy)

* run 'composer install'
* cp config/config.php.example config/config.php
* edit config/config.php to match your environment
* copy bin/phantomjs to the location you set for 
  'phantomjs' in config/config.php, set mode 755
* cd build ; ./build.php
* mv originhealth.phar (install location)
  ie; mv originhealth.phar /usr/local/bin/originhealth


## Usage 

* ./originhealth -h

## Contributors

* "Rodney Mosley (RamJett)" 'rodney at equalit dot ie'

* See: https://wiki.deflect.ca
