#!/bin/bash
cd $(cd -P -- "$(dirname -- "$0")" && pwd -P)
cd ../data/
http-server -p 8000 -a 192.168.1.101 --cors
