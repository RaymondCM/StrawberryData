#!/usr/bin/env python
from __future__ import print_function, division

import rospy
from rasberry_data_capture.RSSaver import RSSaver


def __parse_prefixes(prefixes):
    if prefixes is None or not isinstance(prefixes, str):
        raise ValueError("Incorrect parameters passed for topic prefixes: '{}'".format(prefixes))
    prefixes = prefixes.split(',')
    if len(prefixes) == 0:
        raise ValueError("No parameters passed for topic prefixes: '{}'".format(prefixes))
    return prefixes


def __manual_save():
    rospy.init_node('rasberry_data_saver', anonymous=True)

    topic_prefixes = rospy.get_param("~topic_prefixes", None)
    topic_prefixes = __parse_prefixes(str(topic_prefixes))
    saver = RSSaver(topic_prefixes=topic_prefixes)

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
''