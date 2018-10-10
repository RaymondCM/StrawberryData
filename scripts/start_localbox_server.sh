#!/bin/bash
cd $(cd -P -- "$(dirname -- "$0")" && pwd -P)
cd ../data/label_box
http-server -p 8000 -a "happy" --cors
