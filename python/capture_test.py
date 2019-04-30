import cv2                                # state of the art computer vision algorithms library
import numpy as np                        # fundamental package for scientific computing
import matplotlib.pyplot as plt           # 2D plotting library producing publication quality figures
import pyrealsense2 as rs                 # Intel RealSense cross-platform open-source API
import open3d as opend
import time
import curses
import os
import json
import datetime

print("Environment Ready")
DS5_product_ids = ["0AD1", "0AD2", "0AD3", "0AD4", "0AD5", "0AF6", "0AFE", "0AFF", "0B00", "0B01", "0B03", "0B07","0B3A"]


def find_device_that_supports_advanced_mode() :
    ctx = rs.context()
    ds5_dev = rs.device()
    devices = ctx.query_devices();
    for dev in devices:
        if dev.supports(rs.camera_info.product_id) and str(dev.get_info(rs.camera_info.product_id)) in DS5_product_ids:
            if dev.supports(rs.camera_info.name):
                print("Found device that supports advanced mode:", dev.get_info(rs.camera_info.name))
            return dev
    raise Exception("No device that supports advanced mode was found")


def capture(win):
    win.nodelay(True)
    win.clear()
    while( len(rs.context().devices)<1):
        win.addstr("no devices connected")
        win.refresh()
        time.sleep(50)

    #dev = find_device_that_supports_advanced_mode()
    #advnc_mode = rs.rs400_advanced_mode(dev)
    #jsonObj = json.load(open("parameters.json"))
    #advnc_mode.load_json(str(jsonObj).replace("'", '\"'))
    # print("Advanced mode is", "enabled" if advnc_mode.is_enabled() else "disabled")
    # print("Depth Control: \n", advnc_mode.get_depth_control())
    # print("RSM: \n", advnc_mode.get_rsm())
    # print("RAU Support Vector Control: \n", advnc_mode.get_rau_support_vector_control())
    # print("Color Control: \n", advnc_mode.get_color_control())
    # print("RAU Thresholds Control: \n", advnc_mode.get_rau_thresholds_control())
    # print("SLO Color Thresholds Control: \n", advnc_mode.get_slo_color_thresholds_control())
    # print("SLO Penalty Control: \n", advnc_mode.get_slo_penalty_control())
    # print("HDAD: \n", advnc_mode.get_hdad())
    # print("Color Correction: \n", advnc_mode.get_color_correction())
    # print("Depth Table: \n", advnc_mode.get_depth_table())
    # print("Auto Exposure Control: \n", advnc_mode.get_ae_control())
    # print("Census: \n", advnc_mode.get_census())


    pipeline_1 = rs.pipeline()
    config_1 = rs.config()
    serial_1 = '819112071737'
    config_1.enable_device(serial_1)
    config_1.enable_stream(rs.stream.depth, 1280,720, rs.format.z16, 30)
    config_1.enable_stream(rs.stream.infrared,1, 1280,720, rs.format.y8, 30)
    config_1.enable_stream(rs.stream.infrared,2, 1280,720, rs.format.y8, 30)
    config_1.enable_stream(rs.stream.color,1920,1080,rs.format.rgb8,30)

    #pipeline_2 = rs.pipeline()
    #config_2 = rs.config()
    #serial_2 = '810512060234'
    #config_2.enable_device(serial_2)
    #config_2.enable_stream(rs.stream.depth, 1280,720, rs.format.z16, 30)
    #config_2.enable_stream(rs.stream.infrared,1, 1280,720, rs.format.y8, 30)
    #config_2.enable_stream(rs.stream.infrared,2, 1280,720, rs.format.y8, 30)
    #config_2.enable_stream(rs.stream.color,1920,1080,rs.format.rgb8,30)


    pipeline_1.start(config_1)
    #pipeline_2.start(config_2)

    profile = pipeline_1.get_active_profile()
    #profile_2 = pipeline_2.get_active_profile()


    #Get depth sensor
    depth_sensor = profile.get_device().first_depth_sensor()
    depth_scale = depth_sensor.get_depth_scale()
    depth_sensor.set_option(rs.option.enable_auto_exposure, 1)
    # depth_sensor.set_option(rs.option.depth_units, 0.001)
    # depth_sensor.set_option(rs.option.exposure, 3250.0)


    #depth_sensor_2 = profile_2.get_device().first_depth_sensor()
    #depth_scale = depth_sensor_2.get_depth_scale()
    #depth_sensor_2.set_option(rs.option.enable_auto_exposure, 1)


    #s.set_region_of_interest(roi)

    for i,sensor in enumerate(profile.get_device().query_sensors()):
        if not sensor.is_depth_sensor():
            win.addstr("RGB index : ",i)
            win.addstr("exposure auto rgb  "+str(sensor.get_option(rs.option.enable_auto_exposure))+"\n")
            win.addstr("exposure rgb "+str(sensor.get_option(rs.option.exposure))+"\n")
            win.addstr("RGB Gain = "+str(sensor.get_option(rs.option.gain))+"\n") # Get gain
            #sensor.set_option(rs.option.exposure, 8400.0) # Example to set exposure as 8400.0
            #sensor.set_option(rs.option.gain, 19.0) # Example to set gain as 19.0
            #sensor.set_option(rs.option.emitter_enabled, 0) # Example to disable emitter
        else:
            win.addstr("Depth index : ",i)
            win.addstr("exposure auto Depth  "+str(sensor.get_option(rs.option.enable_auto_exposure))+"\n")
            win.addstr("exposure Depth"+str(sensor.get_option(rs.option.exposure))+"\n")
            win.addstr("Depth Gain = "+str(sensor.get_option(rs.option.gain))) # Get gain
            win.addstr("Depth is emitter enable"+str(sensor.get_option(rs.option.emitter_enabled))+"\n") # Get emitter status
            #sensor.set_option(rs.option.exposure, 8400.0) # Example to set exposure as 8400.0
            #sensor.set_option(rs.option.gain, 19.0) # Example to set gain as 19.0
            sensor.set_option(rs.option.emitter_enabled, 1) # Example to disable emitter
            # roi_sensor = sensor.as_roi_sensor()
            # sensor_roi = roi_sensor.get_region_of_interest()
            # print(sensor_roi.min_x, sensor_roi.max_x, sensor_roi.min_y, sensor_roi.max_y)
            # sensor_roi.min_x, sensor_roi.max_x = 0,int(720)
            # sensor_roi.min_y, sensor_roi.max_y = int(1280/2)-100,int(1280/2)+100
            # roi_sensor.set_region_of_interest(sensor_roi)
            # sensor_roi = roi_sensor.get_region_of_interest()
            # print(sensor_roi.min_x, sensor_roi.max_x, sensor_roi.min_y, sensor_roi.max_y)






    #color_sensor.set_option(rs.option.enable_auto_exposure, 1)


    #Display intrisics for depth
    depth_profile = rs.video_stream_profile(profile.get_stream(rs.stream.depth))
    depth_intrisics = depth_profile.get_intrinsics()
    win.addstr("Depth intrinsics "+str(depth_intrisics)+"\n")

    #Display intrisics for Color
    color_profile = rs.video_stream_profile(profile.get_stream(rs.stream.color))
    color_intrisics = color_profile.get_intrinsics()
    print("Color intrinsics "+str(color_intrisics)+"\n")
    


    key=""

    min_x,max_x = 0,int(1280-1)
    min_y,max_y = int(720/2),int(720-1)
    try:
        while True:
            frameset1 = pipeline_1.wait_for_frames()
            color_frame = frameset1.get_color_frame()
            color = np.asanyarray(color_frame.get_data())
            colorizer = rs.colorizer()
            align = rs.align(rs.stream.color)
            frameset = align.process(frameset1)
            depth_frame = frameset.get_depth_frame()

            if(depth_frame.supports_frame_metadata(rs.frame_metadata_value.actual_exposure)):
                if(depth_sensor.get_option(rs.option.enable_auto_exposure) == 1):
                    win.clear()
                    exposure_auto = depth_frame.get_frame_metadata(rs.frame_metadata_value.actual_exposure)
                    win.addstr("exposure value from meta"+str(exposure_auto)+"\n")
                else:
                    win.erase()
                    win.addstr("exposure value"+str(depth_sensor.get_option(rs.option.exposure))+"\n")

            #frameset2 = pipeline_2.wait_for_frames()
            #color_frame_2 = frameset2.get_color_frame()
            #color_2 = np.asanyarray(color_frame_2.get_data())
            #colorizer = rs.colorizer()
            #align = rs.align(rs.stream.color)
            #frameset_2 = align.process(frameset2)
            #aligned_depth_frame_2 = frameset_2.get_depth_frame()
            #colorized_depth_2 = np.asanyarray(colorizer.colorize(aligned_depth_frame_2).get_data())
            #images = np.hstack((color_2, colorized_depth_2))
            #images = cv2.resize(images, (1280 *2 , 720), interpolation=cv2.INTER_LINEAR)
            #cv2.imshow("depth and rgb D415",images)
            #cv2.waitKey(10)



            aligned_depth_frame = frameset.get_depth_frame()
            aligned_infrared_frame = frameset.get_infrared_frame(1)
            aligned_infrared_frame_2 = frameset.get_infrared_frame(2)

            #spatial = rs.spatial_filter()
            #spatial.set_option(rs.option.holes_fill, 3)
            #filtered_depth = spatial.process(aligned_depth_frame)
            #hole_filling = rs.hole_filling_filter()
            #filtered_depth = hole_filling.process(aligned_depth_frame)

            colorized_depth = np.asanyarray(colorizer.colorize(aligned_depth_frame).get_data())
            aligned_depth = np.asanyarray(aligned_depth_frame.get_data())

            infrared = np.expand_dims(np.asanyarray(aligned_infrared_frame.get_data()),2)
            infrared = np.concatenate( (infrared,infrared,infrared),2)
            infra1 = np.asanyarray(aligned_infrared_frame.get_data())
            infra2 = np.asanyarray(aligned_infrared_frame_2.get_data())
            #stereo = cv2.StereoBM_create(numDisparities=16, blockSize=15)
            #disparity = stereo.compute(infra1,infra2)
            #image = np.hstack((infra1,infra2,disparity))
            #cv2.imshow("disparity",image)
            #cv2.waitKey(500)

            roi_sensor = profile.get_device().query_sensors()[0].as_roi_sensor()
            sensor_roi = roi_sensor.get_region_of_interest()
            sensor_roi.min_x, sensor_roi.max_x = min_x,max_x
            sensor_roi.min_y, sensor_roi.max_y = min_y,max_y
            roi_sensor.set_region_of_interest(sensor_roi)
            cv2.rectangle(infra1, (sensor_roi.min_x, sensor_roi.min_y), (sensor_roi.max_x, sensor_roi.max_y), (255, 255, 0), 5)

            #roi_sensor_2 = profile_2.get_device().query_sensors()[0].as_roi_sensor()
            #sensor_roi_2 = roi_sensor_2.get_region_of_interest()
            #sensor_roi_2.min_x, sensor_roi_2.max_x = 0,int(infrared.shape[1]-1)
            #sensor_roi_2.min_y, sensor_roi_2.max_y = int(infrared.shape[0]/2)-100,int(infrared.shape[0]/2)+100
            #roi_sensor.set_region_of_interest(sensor_roi_2)


            #print(infrared.shape,color.shape,np.asanyarray(aligned_depth_frame.get_data()).astype(np.float).shape)
            images = np.hstack((color, colorized_depth))
            images = cv2.resize(images, (1280, 720), interpolation=cv2.INTER_LINEAR)
            cv2.imshow("depth and rgb D345",images)
            cv2.imshow("Infrared D345",infra1)
            cv2.waitKey(10)

            try:
                key = win.getkey()
                win.addstr("Detected key:")
                win.addstr(str(key))
                if str(key)=="s":
                    print('Saving images')
                    now = str(datetime.datetime.now())
                    os.mkdir(now)
                    cv2.imwrite(now+'/rgb.png',color)
                    cv2.imwrite(now+'/colorized_depth.png',colorized_depth)
                    #cv2.imwrite('rgb2.png',color_2)
                    #cv2.imwrite('colorized_depth2.png',colorized_depth_2)
                    cv2.imwrite(now+'/IR1.png',infra1)
                    cv2.imwrite(now+'/IR2.png',infra2)
                    cv2.imwrite(now+'/aligned_depth.png',aligned_depth)
                elif str(key)=="e":
                    if(depth_frame.supports_frame_metadata(rs.frame_metadata_value.actual_exposure)):
                        win.clear()
                        exposure_auto = depth_frame.get_frame_metadata(rs.frame_metadata_value.actual_exposure)
                        win.addstr("exposure value from meta"+str(exposure_auto)+"\n")
                        if(depth_sensor.get_option(rs.option.enable_auto_exposure) == 1):
                            depth_sensor.set_option(rs.option.enable_auto_exposure, 0)
                            depth_sensor.set_option(rs.option.exposure, exposure_auto)
                        else:
                            depth_sensor.set_option(rs.option.enable_auto_exposure, 1)
                    else:
                        win.addstr("Cannot access metada"+"\n")
                ####  Move ROI
                #### r = up
                #### f = down
                #### d = left
                #### g = right
                #### k = reduce height
                #### l = reduce width
                #### o = augment height
                #### p = augment width
                elif str(key)=="r":
                    if(min_y-10 > 0 ):
                        min_y = min_y - 10
                        max_y = max_y - 10
                elif str(key)=="f":
                    if(max_y+10 < int(infrared.shape[0]-1) ):
                        min_y = min_y + 10
                        max_y = max_y + 10
                elif str(key)=="d":
                    if(min_x-10 > 0 ):
                        min_x = min_x - 10
                        max_x = max_x - 10
                elif str(key)=="g":
                    if(max_x + 10 < int(infrared.shape[1]-1) ):
                        min_x = min_x + 10
                        max_x = max_x + 10
                elif str(key)=="l":
                    if(max_x-min_x >10):
                        min_x = min_x + 10
                        max_x = max_x -10
                elif str(key)=="k":
                    if(max_y-min_y >10):
                        min_y = min_y + 10
                        max_y = max_y -10

                elif str(key)=="p":
                    if(max_x +10 < int(infrared.shape[1]-1)):
                        max_x = max_x + 10
                    if(min_x-10 > 0 ):
                        min_x = min_x - 10
                elif str(key)=="o":
                    if(max_y + 10 < int(infrared.shape[0]-1)):
                        max_y = max_y + 10
                    if(min_y - 10 > 0 ):
                        min_y = min_y - 10
                elif str(key)=="q":
                    break


            except Exception as e:
               # No input
               pass
    finally:
        pipeline_1.stop()



    color_focal_length_1280 = (color_intrisics.fx,color_intrisics.fy)
    color_principal_point_1280 = (color_intrisics.ppx,color_intrisics.ppy)

    depth_focal_length_848 = (427.8556,427.8556)
    depth_principal_point_848 = (426.9114,250.9030)

    depth =  aligned_depth#  np.asanyarray(aligned_depth.get_data()).astype(np.float)
    color = color.astype(np.float)/255

    points = []
    colors = []
    for depth_y in range(0,depth.shape[0]):
        for depth_x in range(0, depth.shape[1] ):
        ## depth to meters
            z =  depth[depth_y][depth_x]  / 1000
            if(z!=0 and z <=10.0):
                x = (depth_x  - color_principal_point_1280[0] ) * z  /  color_focal_length_1280[0]
                y = (depth_y  - color_principal_point_1280[1] ) * z  /  color_focal_length_1280[1]
                point = np.array([x,y,z,1.0])
                point = point[:3]/ point[3]
                points.append(point * np.array([1,-1,-1]))
                colors.append(color[depth_y][depth_x])
    points = np.array(points)
    xcoords = [p[0] for p in points]
    ycoords = [p[1] for p in points]
    zcoords = [p[2] for p in points]
    nb = len(xcoords)
    centroid = np.array([np.sum(xcoords) / nb, np.sum(ycoords) / nb, np.sum(zcoords) / nb])
    points = points - centroid
    pcd = opend.PointCloud()
    pcd.points = opend.Vector3dVector(points)
    pcd.colors = opend.Vector3dVector(np.array(colors))
    # newptc = opend.voxel_down_sample(pcd, voxel_size = 0.0024 )
    opend.draw_geometries([pcd])

curses.wrapper(capture)
