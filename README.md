autodeflect
========

Autodeflect is an automation system for DIY installations of the
[Deflect](https://deflect.ca) anti-DDoS system.

Deflect is a lightweight, low-cost anti-DDoS content distribution
network using reverse proxies to absorb and mitigate DDoS attacks on
websites. The infrastructure is comprised of many parts, with [Apache
Traffic Server](https://trafficserver.apache.org/) being a central
component used for caching resources and serving them. 

Autodeflect is a system for writing out the dynamic components of a
Deflect configuration. This comprises:
* awstats configuration entries
* Apache Traffic Server remap files
* Bind-style zone file information (designed to be used with [Edgemanage](https://github.com/equalitie/edgemanage) for robust serving of content when servers experience instability or become unavailable). 
* Nagios configuration for monitoring origin servers
* Per-site configuration rules for the [Banjax](https://github.com/equalitie/banjax) mitigation platform - both the old-style libconfig-based file and the current YAML-based configuration. 

Configuration 
-------

Global configuration of controller-side elements is accomplished via
variables in ```site.yml```. Comments document the majority of this
configuration.

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

