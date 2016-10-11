## Installation

Requires:
* php-5.4+
* composer
* http webserver
* php-pear
* libyaml-dev
* php5-dev

* run 'composer install'
* cp config.php.example config.php
* edit config.php to match your environment


Install pecl yaml for php yaml support
<pre>
apt-get install php-pear libyaml-dev php5-dev
pecl install yamL
sh -c "echo 'extension=yaml.so' >> /etc/php5/mods-available/yaml.ini"
php5enmod yaml
</pre>

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
