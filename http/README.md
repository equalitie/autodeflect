## Installation

Requires:
* php-5.4+
* composer
* http webserver

* run 'composer install'
* cp config.php.example config.php
* edit config.php to match your environment

Apache:

example config:<pre>

DocumentRoot /path/to/here/public
<Directory /path/to/here/public>
    Options All
    AllowOverride All
    Order allow,deny
    allow from all
</Directory> </pre>


nginx:


## Usage 

## Contributors

* "Rodney Mosley (RamJett)" 'rodney at equalit dot ie'

* See: https://wiki.deflect.ca
