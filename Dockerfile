# This is just a stub, not fully working yet, but general stub to dockerise components

FROM lcasuol/lcas-docker:xenial-base

WORKDIR workspace/src

COPY . rasberry_data_capture

RUN apt-get update \
  && rosdep update \
  && rosdep install --from-paths rasberry_data_capture -i -y

#RUN apt-get install ros-kinetic-catkin
#WORKDIR workspace
#RUN catkin_make