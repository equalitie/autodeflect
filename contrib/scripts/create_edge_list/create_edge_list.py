#!/usr/bin/env python

import os
import glob
import socket
import sys

FAKE_LIST="/usr/local/deflect/etc/whitelist.extra"
EDGELIST_GLOB="/etc/edgemanage/edges/*"

def main(): 
    
    ip_list = []
    with open(FAKE_LIST) as fake_f: 
        ip_list += fake_f.read().strip().split("\n")

    for edgelist_path in glob.glob(EDGELIST_GLOB): 
        if os.path.isfile(edgelist_path): 
            with open(edgelist_path) as edgelist_f: 
                for edgename in edgelist_f.read().strip().split("\n"): 
                    if edgename.startswith("#"):
                        # skip commented edges
                        continue
                    try:
                        edge_ip = socket.gethostbyname(edgename)
                    except:
                        sys.stderr.write("Problem with getting IP for %s\n" % edgename)
                        continue
                    if edge_ip not in ip_list: 
                        ip_list.append(edge_ip)

    print "\n".join(sorted(ip_list))

if __name__ == "__main__":
    main()
