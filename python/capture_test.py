import cv2                                # state of the art computer vision algorithms library
import numpy as np                        # fundamental package for scientific computing
import matplotlib.pyplot as plt           # 2D plotting library producing publication quality figures
import pyrealsense2 as rs                 # Intel RealSense cross-platform open-source API
import open3d as opend
import time
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

dev = find_device_that_supports_advanced_mode()
advnc_mode = rs.rs400_advanced_mode(dev)

print("Advanced mode is", "enabled" if advnc_mode.is_enabled() else "disabled")
print("Depth Control: \n", advnc_mode.get_depth_control())
print("RSM: \n", advnc_mode.get_rsm())
print("RAU Support Vector Control: \n", advnc_mode.get_rau_support_vector_control())
print("Color Control: \n", advnc_mode.get_color_control())
print("RAU Thresholds Control: \n", advnc_mode.get_rau_thresholds_control())
print("SLO Color Thresholds Control: \n", advnc_mode.get_slo_color_thresholds_control())
print("SLO Penalty Control: \n", advnc_mode.get_slo_penalty_control())
print("HDAD: \n", advnc_mode.get_hdad())
print("Color Correction: \n", advnc_mode.get_color_correction())
print("Depth Table: \n", advnc_mode.get_depth_table())
print("Auto Exposure Control: \n", advnc_mode.get_ae_control())
print("Census: \n", advnc_mode.get_census())


pipeline_1 = rs.pipeline()
config_1 = rs.config()
serial_1 = '819112071737'
config_1.enable_device(serial_1)
config_1.enable_stream(rs.stream.depth, 1280,720, rs.format.z16, 30)
config_1.enable_stream(rs.stream.infrared, 1280,720, rs.format.y8, 30)
config_1.enable_stream(rs.stream.color,1280,720,rs.format.rgb8,30)

#config_1.enable_record_to_file('test1.bag')


pipeline_1.start(config_1)

profile = pipeline_1.get_active_profile()

#Get depth sensor
depth_sensor = profile.get_device().first_depth_sensor()
depth_scale = depth_sensor.get_depth_scale()

depth_sensor.set_option(rs.option.enable_auto_exposure, 1)
#depth_sensor.set_option(rs.option.exposure, 250.0)

s = profile.get_device().first_roi_sensor()
roi = s.get_region_of_interest()
print(roi.min_x,roi.min_x,roi.max_x,roi.max_y)
roi.min_x = 0
roi.min_y = 0
roi.max_x = 1280
roi.max_y = 720
print(roi.min_x,roi.min_x,roi.max_x,roi.max_y)
#s.set_region_of_interest(roi)

for i,sensor in enumerate(profile.get_device().query_sensors()):
	if not sensor.is_depth_sensor():
		print("RGB index : ",i)
		print("exposure auto rgb  ",sensor.get_option(rs.option.enable_auto_exposure))
		print("exposure rgb",sensor.get_option(rs.option.exposure))
		print("RGB Gain = ",sensor.get_option(rs.option.gain)) # Get gain
		#sensor.set_option(rs.option.exposure, 8400.0) # Example to set exposure as 8400.0
		#sensor.set_option(rs.option.gain, 19.0) # Example to set gain as 19.0
		#sensor.set_option(rs.option.emitter_enabled, 0) # Example to disable emitter
	else:
		print("Depth index : ",i)
		print("exposure auto Depth  ",sensor.get_option(rs.option.enable_auto_exposure))
		print("exposure Depth",sensor.get_option(rs.option.exposure))
		print("Depth Gain = ",sensor.get_option(rs.option.gain)) # Get gain
		print("Depth is emitter enable",sensor.get_option(rs.option.emitter_enabled)) # Get emitter status
		#sensor.set_option(rs.option.exposure, 8400.0) # Example to set exposure as 8400.0
		#sensor.set_option(rs.option.gain, 19.0) # Example to set gain as 19.0
		sensor.set_option(rs.option.emitter_enabled, 1) # Example to disable emitter
		roi_sensor = sensor.as_roi_sensor()
		sensor_roi = roi_sensor.get_region_of_interest()
		print(sensor_roi.min_x, sensor_roi.max_x, sensor_roi.min_y, sensor_roi.max_y)
		sensor_roi.min_x, sensor_roi.max_x = 0, 10
		sensor_roi.min_y, sensor_roi.max_y = 0, 10
		roi_sensor.set_region_of_interest(sensor_roi)
		sensor_roi = roi_sensor.get_region_of_interest()
		print(sensor_roi.min_x, sensor_roi.max_x, sensor_roi.min_y, sensor_roi.max_y)






#color_sensor.set_option(rs.option.enable_auto_exposure, 1)


#Display intrisics for depth
depth_profile = rs.video_stream_profile(profile.get_stream(rs.stream.depth))
depth_intrisics = depth_profile.get_intrinsics()
print("Depth intrinsics ",depth_intrisics)

#Display intrisics for Color
color_profile = rs.video_stream_profile(profile.get_stream(rs.stream.color))
color_intrisics = color_profile.get_intrinsics()
print("Color intrinsics ",color_intrisics)


while(1):
#	print(depth_sensor.get_option(rs.option.exposure))
	frameset = pipeline_1.wait_for_frames()
	color_frame = frameset.get_color_frame()
	align = rs.align(rs.stream.color)
	depth_frame = frameset.get_depth_frame()
	color = np.asanyarray(color_frame.get_data())
	plt.rcParams["axes.grid"] = False
	plt.rcParams['figure.figsize'] = [12, 6]
	colorizer = rs.colorizer()
	align = rs.align(rs.stream.color)
	frameset = align.process(frameset)

	aligned_depth_frame = frameset.get_depth_frame()
	aligned_infrared_frame = frameset.get_infrared_frame()
	spatial = rs.spatial_filter()
	spatial.set_option(rs.option.holes_fill, 3)
	filtered_depth = spatial.process(aligned_depth_frame)
	hole_filling = rs.hole_filling_filter()
	filtered_depth = hole_filling.process(filtered_depth)
	colorized_depth = np.asanyarray(colorizer.colorize(filtered_depth).get_data())
	infrared = np.expand_dims(np.asanyarray(aligned_infrared_frame.get_data()),2)
	infrared = np.concatenate( (infrared,infrared,infrared),2)
	
	##set roi area around the darkest infrared value
	index_min = np.unravel_index(np.argmin(infrared[:,:,0]), infrared.shape)
	roi_sensor = profile.get_device().query_sensors()[0].as_roi_sensor()
	sensor_roi = roi_sensor.get_region_of_interest()
	sensor_roi.min_x, sensor_roi.max_x = int(index_min[1]),int( index_min[1] + 1) if index_min[1] +1 < infrared.shape[1] else int(infrared.shape[1]-1)
	sensor_roi.min_y, sensor_roi.max_y = int(index_min[0]), int( index_min[0] + 1) if index_min[0] +1 <infrared.shape[0] else int(infrared.shape[0]-1)
	roi_sensor.set_region_of_interest(sensor_roi)
	sensor_roi = roi_sensor.get_region_of_interest()


	#print(infrared.shape,color.shape,np.asanyarray(aligned_depth_frame.get_data()).astype(np.float).shape)
	images = np.hstack((color, colorized_depth,infrared))
	cv2.imshow("depth and rgb",images)
	cv2.waitKey(500)
# Cleanup:
#pipeline_1.stop()

color_focal_length_1280 = (color_intrisics.fx,color_intrisics.fy)
color_principal_point_1280 = (color_intrisics.ppx,color_intrisics.ppy)

depth_focal_length_848 = (427.8556,427.8556)
depth_principal_point_848 = (426.9114,250.9030)

extrinsic_234 = np.asarray([[0.999992,-0.002436,0.012152,0.019]
                        ,[0.002471,0.9999,-0.00286,-9.7e-3]
                        ,[-0.012145,0.002987,0.99992,0.0002839]
                        ,[0,0,0,1]])
extrinsic = extrinsic_234
inverse_extrinsic = np.linalg.inv(extrinsic)

depth =	np.asanyarray(aligned_depth_frame.get_data()).astype(np.float)
color = color.astype(np.float)/255



#### apply post processing
# spatial = rs.spatial_filter()
# spatial.set_option(rs.option.holes_fill, 3)
# filtered_depth = spatial.process(aligned_depth_frame)
# hole_filling = rs.hole_filling_filter()
# filtered_depth = hole_filling.process(filtered_depth)
# colorized_depth = np.asanyarray(colorizer.colorize(filtered_depth).get_data())
# plt.imshow(colorized_depth)
#
# plt.show()


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
            point = np.dot(inverse_extrinsic,point)
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
