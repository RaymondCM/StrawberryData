#!/bin/bash

#Change to script directory
cd $(cd -P -- "$(dirname -- "$0")" && pwd -P)

#Change to data root directory
cd ..

d_count=$(find . -iname "depth_16UC1.png" | wc -l)
cd_count=$(find . -iname "colourised_depth_8UC3.png" | wc -l)
rgb_count=$(find . -iname "rgb_8UC3.png" | wc -l)
irl_count=$(find . -iname "ir_left_8UC1.png" | wc -l)
irr_count=$(find . -iname "ir_right_8UC1.png" | wc -l)
pc_count=$(find . -iname "point_cloud.ply" | wc -l)

echo "File Name                Count"
echo "------------------------------"
echo "Depth Count              $d_count"
echo "Colourised Depth Count   $cd_count"
echo "RGB Count                $rgb_count"
echo "IR Left Count            $irl_count"
echo "IR Right Count           $irr_count"
echo "Point Cloud Count        $pc_count"

