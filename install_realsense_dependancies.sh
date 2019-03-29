#!/usr/bin/env bash
cd $(cd -P -- "$(dirname -- "$0")" && pwd -P)
sudo true

kernel_version=$(uname -r)
lib_version="2.19.0"
ros_lib_version="2.2.1"

case ${kernel_version} in "4.4"*|"4.8"*|"4.10"*|"4.13"*|"4.15"*|"4.16"*)
        echo "Kernel Version '$kernel_version' matches supported '4.[4,8,10,13,15,16].*'"
        echo "Installing librealsense v${lib_version}"
        sleep 1

        read -r -p "${1:-Have you unplugged all RealSense devices? [y/N]} " response
        case "$response" in
            [yY][eE][sS]|[yY])
                ;;
            *)
                (>&2 echo  "You must remove all connected RealSense devices.")
                exit 1
                ;;
        esac

        sudo apt-get install git libssl-dev libusb-1.0-0-dev pkg-config libgtk-3-dev -y
        sudo apt-get install libglfw3-dev -y

        wget https://github.com/IntelRealSense/librealsense/archive/v${lib_version}.tar.gz
        tar xvzf v${lib_version}.tar.gz
        cd librealsense-${lib_version}/ || exit 1
        sudo cp config/99-realsense-libusb.rules /etc/udev/rules.d/
        sudo udevadm control --reload-rules && udevadm trigger
        ./scripts/patch-realsense-ubuntu-lts.sh
        mkdir build && cd build
        cmake ../ -DBUILD_EXAMPLES=true
        sudo make uninstall && make clean && make -j4 && sudo make install

        cd ../..
        sudo rm -r v${lib_version}.tar.gz librealsense-${lib_version}/

        echo "Installing librealsense ROS camera wrapper v${ros_lib_version}"
        sleep 1

        cd ../
        wget https://github.com/intel-ros/realsense/archive/${ros_lib_version}.tar.gz
        tar xvzf ${ros_lib_version}.tar.gz
        rm ${ros_lib_version}.tar.gz
        cd ..
        catkin_make clean
        catkin_make -DCATKIN_ENABLE_TESTING=False -DCMAKE_BUILD_TYPE=Release
        catkin_make install

        ;;
    *)
        (>&2 echo  "Your kernel version '$kernel_version' is different to the kernel that this script was tested with.")
        (>&2 echo -e "\tPlease refer to https://github.com/IntelRealSense/librealsense/blob/master/doc/installation.md")
        exit 1
        ;;
esac
