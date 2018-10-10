#!/bin/bash
cd $(cd -P -- "$(dirname -- "$0")" && pwd -P)
cd ../data/
http-server -p 8000 -a "happy" --cors
