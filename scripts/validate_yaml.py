#!/usr/bin/env python

# validate a yaml file by loading it. No fancy stuff


import os
import sys
import yaml

def main(filename): 
    try:
        file_content = open(filename).read()
        yaml_data = yaml.load(file_content)
    except Exception as e: 
        sys.stderr.write("Failed to load file %s - invalid YAML: %s \n" % (filename, str(e)))
        print file_content
        sys.exit(3)

if __name__ == "__main__":
    if len(sys.argv) != 2: 
        sys.stderr.write("Usage: validate_yaml.py filename\n")
        sys.exit(1)
    filename = sys.argv[1]
    if not os.path.exists(filename): 
        sys.stderr.write("File %s doesn't exist!\n" % filename)
        sys.exit(2)

    main(filename)
