#ifndef STRAWBERRYDATA_REALSENSED400_H
#define STRAWBERRYDATA_REALSENSED400_H

#include <string>
#include <librealsense2/rs.hpp>
#include <opencv2/opencv.hpp>
#include "Strawberry.hpp"

class RealSenseD400 {
public:
    explicit RealSenseD400(rs2::device dev, bool gui_enabled_ = true);
    ~RealSenseD400();
    void PrintDeviceInfo();
    void StabilizeExposure(int stabilization_window = 30);
    void Stream();
private:
    // Device
    rs2::device dev_;
    rs2::depth_sensor depth_sensor_;
    float depth_sensor_scale_;
    std::string serial_number_;

    // Pipeline configuration
    rs2::config cfg;
    rs2::pipeline pipe_;
    rs2::pipeline_profile selection;

    // Point cloud
    rs2::colorizer color_map;
    rs2::pointcloud pc_;

    // OpenCV Windows
    std::string win_colour_ = "Colour", win_ir_ = "IR (Left, Right)", win_depth_ = "Depth (Uncoloured, Colourised)";
    char input_ = '\0';

    // Frame information
    rs2::frameset frames_;
    rs2::video_frame depth_, colour_, lir_, rir_, c_depth_;

    rs2::points point_cloud_;

    // OpenCV Frames
    cv::Mat colour_mat_, depth_mat_, c_depth_mat_, lir_mat_, rir_mat_;

    // Preview Frames
    cv::Mat lrir_mat, cd_depth_mat, depth_mat_8bit;

    // Dataset structure
    Strawberry::DataStructure data_structure_;

    // Visualisation flags
    bool gui_enabled_;

    // Utility
    void WriteVideoFrameMetaData(const std::string &file_name, rs2::video_frame &frame);
    bool WindowsAreOpen();

    void WriteData();

    void Visualise();
};

#endif //STRAWBERRYDATA_REALSENSED400_H
