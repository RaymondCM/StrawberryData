#!/usr/bin/env python
from __future__ import print_function, division

import cv2
import numpy as np
import rospy
from cv_bridge import CvBridge
from sensor_msgs.msg import Image


class Viewer:
    def __init__(self, depth_topic="/rsense_camera/aligned_depth/image_raw",
                 live_topic="/rsense_camera/color/image_raw",
                 summary_topic="/rasberry_data_capture/summary"):
        self.summary_image = np.zeros((1080 * 2, 1920 * 2, 3)).astype(np.uint8)
        self.live_image = np.zeros((1080, 1920, 3)).astype(np.uint8)
        self.depth_image = np.zeros((1080, 1920, 3)).astype(np.uint8)
        self.image = np.zeros((1080 * 3, 1920 * 2, 3)).astype(np.uint8)

        self.bridge = CvBridge()
        self.live_topic = live_topic
        self.depth_topic = depth_topic
        self.summary_topic = summary_topic
        self.live_feed = rospy.Subscriber(self.live_topic, Image, self.update_live)
        self.depth_feed = rospy.Subscriber(self.depth_topic, Image, self.update_depth)
        self.data_summary = rospy.Subscriber(self.summary_topic, Image, self.update_summary)

    def update_summary(self, data):
        self.summary_image = self.bridge.imgmsg_to_cv2(data, "bgr8")
        self.image[1080:, :, :] = self.summary_image

    def update_live(self, data):
        self.live_image = self.bridge.imgmsg_to_cv2(data, "bgr8")
        self.image[:1080, :1920, :] = self.live_image

    def update_depth(self, data):
        depth_image = self.bridge.imgmsg_to_cv2(data, "16UC1")
        depth_image = (((depth_image - np.min(depth_image)) / np.ptp(depth_image)) * 255).astype(np.uint8)
        self.depth_image = cv2.applyColorMap(depth_image, cv2.COLORMAP_JET)
        self.image[1080:, 1920:, :] = self.depth_image

    def display_loop(self, hz=int(1000 / 3)):
        cv2.namedWindow(self.summary_topic, cv2.WINDOW_NORMAL)
        key = ord(' ')
        while not rospy.is_shutdown() and key != ord('q'):
            cv2.imshow(self.summary_topic, self.image)
            key = cv2.waitKey(hz)


if __name__ == '__main__':
    rospy.init_node("image_viewer", anonymous=True)
    viewer = Viewer()

    try:
        viewer.display_loop()
    except KeyboardInterrupt:
        print("Exiting Node")

    cv2.destroyAllWindows()
