autodeflect
========

Autodeflect is an automation system for DIY installations of the
[Deflect](https://deflect.ca) anti-DDoS CDN system.

Deflect is a lightweight but heavy-duty anti-DDoS content distribution
network that uses low-cost reverse proxies to absorb and mitigate DDoS
attacks on webservers. The infrastructure is comprised of many parts,
with [Apache Traffic Server](https://trafficserver.apache.org/) being a
central component used for caching resources and serving them. 

Autodeflect is a system for writing out the dynamic components of a
Deflect configuration. This comprises:
* Apache Traffic Server remap files
* Bind-style zone file information (designed to be used with [Edgemanage](https://github.com/equalitie/edgemanage) for robust serving of content when servers experience instability or become unavailable). 
* icinga configuration for monitoring origin servers
* site configuration rules for the [Banjax](https://github.com/equalitie/banjax) mitigation platform. 
* Scripted renewal of [Let's Encrypt](https://letsencrypt.org/) TLS certs

Configuration 
-------

Global configuration of controller-side elements is accomplished via
variables. These are created with ```ansible-playbook init.yml```.
Comments document the majority of this configuration.

Setup you inventory in ```config/inventory/inventory```
Then run your playbook. ``````ansible-playbook init.yml```.
Note: You should only do this after autodeflect was install with
```cityhall```

Client configuration (sites protected behind your instance of Deflect)
is accomplished via ```clients.yml```. In the Deflect system this file
is generated via the [Deflect
Dashboard](https://dashboard.deflect.ca). This file can be written by
hand or populated by some automated system. Some Day the Deflect
Dashboard source will be opened, but this is not that day.

Limitations
-------

Autodeflect does *not* write out configuration for a Nagios
installation, an Awstats setup or an Apache Traffic Server
configuration set. Users should supply these configurations themselves
(generally the stock configurations are fine, but vast improvements
can be obtained by tweaking them). In future static configuration
files will be added to this repository.

Requires:
* python-passlib
