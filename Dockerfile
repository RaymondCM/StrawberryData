# This is just a stub, not fully working yet, but general stub to dockerise components

FROM lcasuol/lcas-docker:xenial-base

WORKDIR workspace/src


RUN apt-get update; apt-get dist-upgrade -y; \
  rosdep update 
COPY . rasberry_data_capture
RUN rosdep install --from-paths rasberry_data_capture -i -y
RUN bash -c 'source /opt/ros/kinetic/setup.bash; \
    cd /workspace; catkin_make; \
'
ENTRYPOINT bash -c 'cd /workspace/src/rasberry_data_capture; \
  source /workspace/devel/setup.bash; \
  ./entry-point.sh; \
'
#RUN apt-get install ros-kinetic-catkin
#WORKDIR workspace
#RUN catkin_make