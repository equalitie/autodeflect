#!/usr/bin/env python

import sys

if len(sys.argv) != 2: 
    raise SystemExit("Provide a path to the config file you wish to read")

from pylibconfig import Config
config = Config ()

try:
    config.readFile ( sys.argv[1] )
    print "Config loaded"
    sys.exit(0)
except Exception as e: 
    print "Config failed to read :("
    print e
    sys.exit(1)

