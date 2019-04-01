#!/usr/bin/env bash
cd $(cd -P -- "$(dirname -- "$0")" && pwd -P)
sh /home/raymond/.local/share/JetBrains/Toolbox/apps/PyCharm-P/ch-0/191.6183.50/bin/pycharm.sh &
rviz -d ../config/cameras.rviz &
