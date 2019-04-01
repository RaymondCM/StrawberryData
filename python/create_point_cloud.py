import numpy as np
import png
import open3d as opend
import sys
from PIL import Image
import matplotlib.pyplot as plt

path = sys.argv[1]
depth_raw  =  png.Reader(path+"/aligned_depth.png")
columnCount, rowCount, pngData, metaData = depth_raw.asDirect()
bitDepth=metaData['bitdepth']
planeCount = metaData['planes']
depth_raw = np.vstack(list(pngData))
print(np.amax(depth_raw))
print(np.amin(depth_raw))


color = np.asarray(Image.open(path+"/rgb.png"))
color = color[:,:,::-1]/255.0

color_focal_length = (1386.075684, 1384.652222)
color_principal_point = ( 947.828674, 554.063538)

points = []
colors = []
print(depth_raw.shape)
for depth_y in range(0,rowCount):
    for depth_x in range(0, columnCount ):
        ## depth to meters
        z =  depth_raw[depth_y][depth_x]  / 1000.0
        if(z!=0 and z <= 1.0):
            x = (depth_x  - color_principal_point[0] ) * z  /  color_focal_length[0]
            y = (depth_y  - color_principal_point[1] ) * z  /  color_focal_length[1]
            point = np.array([x,y,z,1.0])
            point = point[:3]/ point[3]
            points.append(point * np.array([1,-1,-1]))
            colors.append(color[depth_y][depth_x])
print(np.array(points).shape)
points = np.array(points)
colors = np.array(colors)
xcoords = [p[0] for p in points]
ycoords = [p[1] for p in points]
zcoords = [p[2] for p in points]
nb = len(xcoords)
centroid = np.array([np.sum(xcoords) / nb, np.sum(ycoords) / nb, np.sum(zcoords) / nb])
points = points - centroid
pcd = opend.PointCloud()
pcd.points = opend.Vector3dVector(points)
pcd.colors = opend.Vector3dVector(colors)
newptc = opend.voxel_down_sample(pcd, voxel_size = 0.0024 )
opend.draw_geometries([newptc])
