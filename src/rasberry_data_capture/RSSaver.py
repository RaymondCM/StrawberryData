#!/usr/bin/env python
from __future__ import print_function, division

import numpy as np
import rospy
from std_msgs.msg import Empty, String
from sensor_msgs.msg import Image, CameraInfo
from realsense2_camera.msg import Extrinsics
from cv_bridge import CvBridge, CvBridgeError
from copy import deepcopy
import datetime
import rospkg
import cv2
import os
import random
import string
import json


class Queue:
    def __init__(self, list_arr):
        self.history = []
        for value in list_arr:
            self.history.append(value)

    def enqueue(self, value):
        self.history.append(value)

    def dequeue(self):
        if len(self.history) > 0:
            return self.history.pop(0)
        else:
            return False

    def add(self, value):
        x = self.dequeue()
        self.enqueue(value)
        return x

    def output(self):
        st = ""
        for value in self.history:
            st = st + " > " + str(value)
        print(st)


class RSSaver:
    def __init__(self, topic_prefixes):
        self.prefixes = topic_prefixes

        self.save_path = os.path.abspath(os.path.join(rospkg.RosPack().get_path('rasberry_data_capture'), "saved_data"))
        self.__create_save_folder()
        self.save_id = 0

        self.bridge = CvBridge()
        self.image_data = {k: {} for k in topic_prefixes}
        self.image_info = {k: {} for k in topic_prefixes}

        self.sensor_fps = 6.0
        self.max_frames_difference = 3.0
        self.max_time_difference = self.max_frames_difference * (1 / self.sensor_fps)
        self.arrival_time = {k: {} for k in topic_prefixes}
        self.frame_status = {k: {} for k in topic_prefixes}

        self.subs = {k: {} for k in self.prefixes}
        self.__create_subs()

        self.log_data_sub = rospy.Subscriber("/rasberry_data_capture/dump", Empty, self.dump)
        self.loc_data_sub = rospy.Subscriber("/current_edge", String, self.dump)

        self.data_summary_pub = rospy.Publisher("/rasberry_data_capture/summary", Image)
        self.depth_queue = Queue([None] * 4)
        self.colour_queue = Queue([None] * 4)

    def __create_subs(self):
        self.subs = {k: {} for k in self.prefixes}
        for pre in self.prefixes:
            # Image topics
            self.subs[pre]["colour"] = rospy.Subscriber("/{}_camera/color/image_raw".format(pre), Image, self.save,
                                                        (pre, "colour", "bgr8"))
            self.subs[pre]["depth_aligned"] = rospy.Subscriber("/{}_camera/aligned_depth_to_color/image_raw".format(pre)
                                                               , Image, self.save, (pre, "depth_aligned", "16UC1"))
            self.subs[pre]["depth"] = rospy.Subscriber("/{}_camera/depth/image_rect_raw".format(pre), Image, self.save,
                                                       (pre, "depth", "16UC1"))
            self.subs[pre]["infra1"] = rospy.Subscriber("/{}_camera/infra1/image_rect_raw".format(pre), Image,
                                                        self.save, (pre, "infra1", "8UC1"))
            self.subs[pre]["infra2"] = rospy.Subscriber("/{}_camera/infra2/image_rect_raw".format(pre), Image,
                                                        self.save, (pre, "infra2", "8UC1"))

            # Camera Info topics (Intrinsic)
            self.subs[pre]["colour_info"] = rospy.Subscriber("/{}_camera/color/camera_info".format(pre), CameraInfo,
                                                             self.save_info, (pre, "colour", "intrinsic"))
            self.subs[pre]["depth_aligned_info"] = rospy.Subscriber(
                "/{}_camera/aligned_depth_to_color/camera_info".format(pre),
                CameraInfo, self.save_info, (pre, "depth_aligned", "intrinsic"))
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
        date = str(datetime.date.today())
        while attempts < 100:
            attempts += 1
            pth = ''.join(random.choice(string.ascii_uppercase + string.digits) for _ in range(n))
            pth = os.path.join(self.save_path, date, pth)
            if not os.path.isdir(pth):
                self.save_path = pth
                return
        raise ValueError("Could not create a save folder for node in '{}'".format(self.save_path))

    def save(self, data, args):
        self.arrival_time[args[0]][args[1]] = rospy.get_rostime()
        self.image_data[args[0]][args[1]] = self.bridge.imgmsg_to_cv2(data, args[2])
        self.frame_status[args[0]][args[1]] = True

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

    def publish_summary(self):
        if all(x is None for x in self.colour_queue.history) or all(x is None for x in self.depth_queue.history):
            return

        height, width = self.colour_queue.history[-1].shape[:2]
        canvas = np.zeros((height * 2, width * 4, 3), dtype=np.uint8)
        positions_rgb = [(0, 0), (0, width * 2), (height, 0), (height, width * 2)]
        positions_depth = [(0, width), (0, width * 3), (height, width), (height, width * 3)]

        for i, rgb in enumerate(self.colour_queue.history):
            if rgb is not None:
                x, y = positions_rgb[i]
                canvas[x:x + height, y:y + width] = rgb

        for i, depth in enumerate(self.depth_queue.history):
            if depth is not None:
                x, y = positions_depth[i]
                depth = (((depth - np.min(depth)) / np.ptp(depth)) * 255).astype(np.uint8)
                depth = cv2.applyColorMap(depth, cv2.COLORMAP_JET)
                canvas[x:x+height, y:y+width] = depth

        self.data_summary_pub.publish(self.bridge.cv2_to_imgmsg(canvas, "bgr8"))
        # if all(x is not None for x in self.colour_queue.history):
        #     cv2.namedWindow("test", cv2.WINDOW_NORMAL)
        #     cv2.imshow("test", canvas)
        #     cv2.waitKey(0)
        #     cv2.destroyAllWindows()

    def dump(self, data=None):
        # Check if source was conditional (if empty just log data)
        if isinstance(data, String):
            data = str(data.data)
            # If string is passed then check if message is 'none' which indicated robot arrived at destination node
            if data != "none":
                return

        # Create state objects to avoid corruption
        image_data = deepcopy(self.image_data)
        image_info = deepcopy(self.image_info)
        frame_status = deepcopy(self.frame_status)
        arrival_time = deepcopy(self.arrival_time)

        # Set all to false on master frame_status (requires it to collect all new frames from this point)
        for node_prefix in self.frame_status.keys():
            self.frame_status[node_prefix] = dict.fromkeys(self.frame_status[node_prefix], False)

        if not os.path.isdir(self.save_path):
            os.makedirs(self.save_path)

        # Check see if new full set of images have arrived (no greater than 4 * (1 / fps) difference
        save_id = os.path.join(self.save_path, "{}_{}".format(self.save_id, '{}.{}'))
        full_image_set = ["colour", "depth", "depth_aligned", "infra1", "infra2"]
        current_time = rospy.get_rostime().to_sec()

        for node_prefix, sensors in image_data.items():
            # Check a full set of images exist
            if not all([s in sensors for s in full_image_set]):
                print("Not all '{}' camera node frames have arrived, cannot save".format(node_prefix))
                continue

            # Check they are all new frames
            if not all(frame_status[node_prefix].values()):
                print("Saved frame set has not changed since last save, cannot save")
                continue

            # Check they have arrived within N seconds (N=(4 * (1/ 6)
            time_stamps = [t.to_sec() for t in arrival_time[node_prefix].values()]
            difference = max(time_stamps) - min(time_stamps)

            if difference > self.max_time_difference:
                print("Frames sync disparity too large at {:.1f}, cannot save".format(self.sensor_fps * difference))
                continue

            if image_info[node_prefix]:
                image_info[node_prefix]["save_time"] = current_time
                info_save_path = save_id.format("{}_camera_info".format(node_prefix), "json")
                print("Saving file '{}'".format(info_save_path))
                with open(info_save_path, 'w') as fp:
                    json.dump(image_info[node_prefix], fp, sort_keys=True, indent=4)

            self.colour_queue.add(sensors["colour"])
            self.depth_queue.add(sensors["depth_aligned"])
            self.publish_summary()

            for sensor_type, sensor_data in sensors.items():
                data_save_path = save_id.format("{}_{}".format(node_prefix, sensor_type), "png")
                print("Saving file '{}'".format(data_save_path))
                cv2.imwrite(data_save_path, sensor_data)

        self.save_id += 1
