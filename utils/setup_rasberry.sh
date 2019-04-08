#!/bin/bash

cd catkin_ws/src/
git clone https://github.com/RaymondKirk/RASberry.git
git clone https://github.com/LCAS/Thorvald.git

# Add LCAS Build Farm
curl -s http://lcas.lincoln.ac.uk/repos/public.key | sudo apt-key add -
sudo apt-add-repository http://lcas.lincoln.ac.uk/ubuntu/main
sudo apt-get update

# Install ROS dependancies
sudo apt install ros-kinetic-strands-*
sudo apt install ros-kinetic-rasberry
sudo apt install ros-kinetic-rosbridge-*
sudo apt install ros-kinetic-rosduct 
sudo apt install ros-kinetic-thorvald
sudo apt install ros-kinetic-urg-node 
sudo apt install ros-kinetic-contact-monitor 
sudo apt install ros-kinetic-eband-local-planner 
sudo apt install ros-kinetic-marvelmind-nav 
sudo apt install ros-kinetic-ira-laser-tools 
sudo apt install ros-kinetic-persistent-topics
