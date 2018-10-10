#!/bin/bash
function check_symlink {
  link=$1
  link_to=$2
  if [ -L ${link_to} ] ; then
     if [ -e ${link_to} ] ; then
        echo "Symlink exists"
     else
        echo "Symlink broken. Recreating."
	sudo rm $link_to
        sudo ln -s $link $link_to
     fi
  elif [ -e ${link_to} ] ; then
     echo "Not a link. Exiting"
     exit 1
  else
     echo "Link missing. Creating."
     sudo ln -s $link $link_to
  fi
}

function safe_install_apt {
  package="$1"
  if [ "" == "$(dpkg-query -W --showformat='${Status}\n' $package | grep "ok installed")" ]; then
    echo "Package $package is not installed. Installing ..."
    sudo apt --assume-yes install $package
  else
    echo "Package $package already exists."
  fi
}

function safe_install_npm {
  package="$1"
  if [ `npm list -g --depth=0 | grep -c $package` -eq 0 ]; then
      echo "Npm package $package is not installed. Installing ..."
      sudo npm install $package -g --no-shrinkwrap
  else
      echo "Npm package $package already exists."
  fi
}

safe_install_apt "nodejs"
safe_install_apt "npm"
check_symlink "/usr/bin/nodejs" "/usr/bin/node"
safe_install_npm "http-server"



