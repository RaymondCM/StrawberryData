#!/bin/bash
cd $(cd -P -- "$(dirname -- "$0")" && pwd -P)

package="python3-virtualenv"
if [ "" == "$(dpkg-query -W --showformat='${Status}\n' $package | grep "ok installed")" ]; then
  echo "Package $package is not installed. Installing ..."
  sudo apt-get --assume-yes install $package
fi

python3 -m virtualenv -p python3 venv
source venv/bin/activate
pip install -r requirements.txt
pip install virtualenvwrapper
source venv/bin/virtualenvwrapper.sh
add2virtualenv $(pwd)
echo $(pwd) > venv/.project
echo "You should upgrade the package requirements file as necessary. Via pip freeze > requirements.txt"
