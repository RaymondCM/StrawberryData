#!/usr/bin/env python
from __future__ import print_function, division

import rospy
from std_msgs.msg import Empty
from sensor_msgs.msg import Image, CameraInfo
from realsense2_camera.msg import Extrinsics
from cv_bridge import CvBridge, CvBridgeError
import rospkg
import cv2
import os
import random
import string
import json


class RSSaver:
    def __init__(self, topic_prefixes=None):
        if topic_prefixes is None:
            topic_prefixes = ["top", "mid", "bot"]
        self.prefixes = topic_prefixes

        self.save_path = os.path.abspath(os.path.join(rospkg.RosPack().get_path('rasberry_data_capture'), "saved_data"))
        self.__create_save_folder()
        self.save_id = 0

        self.bridge = CvBridge()
        self.image_data = {k: {} for k in topic_prefixes}
        self.image_info = {k: {} for k in topic_prefixes}
        self.seq = {k: {} for k in topic_prefixes}

        self.subs = {k: {} for k in self.prefixes}
        self.__create_subs()

        self.log_data_sub = rospy.Subscriber("/rasberry_data_capture/dump", Empty, self.dump)

    def __create_subs(self):
        self.subs = {k: {} for k in self.prefixes}
        for pre in self.prefixes:
            # Image topics
            self.subs[pre]["colour"] = rospy.Subscriber("/{}_camera/color/image_raw".format(pre), Image, self.save,
                                                        (pre, "colour", "bgr8"))
            self.subs[pre]["depth"] = rospy.Subscriber("/{}_camera/aligned_depth_to_color/image_raw".format(pre), Image,
                                                       self.save, (pre, "depth", "16UC1"))
            self.subs[pre]["infra1"] = rospy.Subscriber("/{}_camera/infra1/image_rect_raw".format(pre), Image,
                                                        self.save, (pre, "infra1", "8UC1"))
            self.subs[pre]["infra2"] = rospy.Subscriber("/{}_camera/infra2/image_rect_raw".format(pre), Image,
                                                        self.save, (pre, "infra2", "8UC1"))

            # Camera Info topics (Intrinsic)
            self.subs[pre]["colour_info"] = rospy.Subscriber("/{}_camera/color/camera_info".format(pre), CameraInfo,
                                                             self.save_info, (pre, "colour", "intrinsic"))
            self.subs[pre]["depth_info"] = rospy.Subscriber("/{}_camera/depth/camera_info".format(pre), CameraInfo,
                                                            self.save_info, (pre, "depth", "intrinsic"))
            self.subs[pre]["infra1_info"] = rospy.Subscriber("/{}_camera/infra1/camera_info".format(pre), CameraInfo,
                                                             self.save_info, (pre, "infra1", "intrinsic"))
            self.subs[pre]["infra2_info"] = rospy.Subscriber("/{}_camera/infra2/camera_info".format(pre), CameraInfo,
                                                             self.save_info, (pre, "infra2", "intrinsic"))

            # Camera Info (Extrinsic)
            self.subs[pre]["depth_to_color"] = rospy.Subscriber("/{}_camera/extrinsics/depth_to_color".format(pre),
                                                                Extrinsics, self.save_info,
                                                                (pre, "colour", "extrinsic"))
            self.subs[pre]["depth_to_infra1"] = rospy.Subscriber("/{}_camera/extrinsics/depth_to_infra1".format(pre),
                                                                 Extrinsics, self.save_info,
                                                                 (pre, "infra1", "extrinsic"))
            self.subs[pre]["depth_to_infra2"] = rospy.Subscriber("/{}_camera/extrinsics/depth_to_infra2".format(pre),
                                                                 Extrinsics, self.save_info,
                                                                 (pre, "infra2", "extrinsic"))

    def __create_save_folder(self, n=5):
        attempts = 0
        while attempts < 10:
            pth = ''.join(random.choice(string.ascii_uppercase + string.digits) for _ in range(n))
            pth = os.path.join(self.save_path, pth)
            if not os.path.isdir(pth):
                self.save_path = pth
                return
            attempts += 1
        raise ValueError("Could not create a save folder for node in '{}'".format(self.save_path))

    def save(self, data, args):
        self.image_data[args[0]][args[1]] = self.bridge.imgmsg_to_cv2(data, args[2])
        if args[1] not in self.seq[args[0]]:
            self.seq[args[0]][args[1]] = 0
        self.seq[args[0]][args[1]] += 1

    def save_info(self, data, args):
        if args[1] not in self.image_info[args[0]]:
            self.image_info[args[0]][args[1]] = {}
        data_dict = {}

        if isinstance(data, CameraInfo):
            parameters = [list(a) for a in [data.D, data.K, data.R, data.P]]
            data_dict["D"], data_dict["K"], data_dict["R"], data_dict["P"] = parameters
        elif isinstance(data, Extrinsics):
            parameters = [list(a) for a in [data.rotation, data.translation]]
            data_dict["rotation"], data_dict["translation"] = parameters
        else:
            raise ValueError("Unsupported argument in info callback '{}'[2]".format(args))

        self.image_info[args[0]][args[1]][args[2]] = data_dict

    def dump(self, data=None):
        if not os.path.isdir(self.save_path):
            os.makedirs(self.save_path)

        save_id = os.path.join(self.save_path, "{}_{}".format(self.save_id, '{}.{}'))

        for node_prefix, sensors in self.image_data.items():
            if self.image_info[node_prefix]:
                info_save_path = save_id.format("{}_camera_info".format(node_prefix), "json")
                print("Saving file '{}'".format(info_save_path))
                with open(info_save_path, 'w') as fp:
                    json.dump(self.image_info[node_prefix], fp, sort_keys=True, indent=4)

            for sensor_type, sensor_data in sensors.items():
                sequence_data = self.seq[node_prefix][sensor_type]
                data_save_path = save_id.format("{}_{}_{}".format(node_prefix, sensor_type, sequence_data), "png")
                print("Saving file '{}'".format(data_save_path))
                cv2.imwrite(data_save_path, sensor_data)

        self.save_id += 1


def __manual_save():
    rospy.init_node('rasberry_data_saver', anonymous=True)
    saver = RSSaver()

    keep_running = True
    try:
        while keep_running and not rospy.core.is_shutdown():
            key = raw_input("Press (s) to save the RealSense camera data: ")
            if len(key) == 0:
                continue

            key = key[0].lower()
            if key == "s":
                saver.dump()
            elif key == "q":
                keep_running = False
    except KeyboardInterrupt:
        print("Exiting Node")


if __name__ == '__main__':
    __manual_save()
