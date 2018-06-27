#ifndef STRAWBERRYDATA_STRAWBERRY_H
#define STRAWBERRYDATA_STRAWBERRY_H

#include <string>
#include <chrono>
#include <ctime>

#include <librealsense2/rs.hpp>
#include <boost/filesystem.hpp>
#include <iomanip>

enum class RsType : int { DEPTH, COLOURED_DEPTH, COLOUR, IR, IR_LEFT, IR_RIGHT, POINT_CLOUD };

namespace Strawberry {

    class DataStructure {
    public:
        explicit DataStructure(const rs2::device &device, std::string path_prefix = "./");
        explicit DataStructure(const char * device_serial_number, std::string path_prefix = "./");
        explicit DataStructure(std::string device_serial_number, std::string path_prefix = "./");

        const void UpdatePathPrefix(std::string path_prefix);
        const void UpdateFolderPaths(bool stop_at_folder_depth = false);
        const std::string FilePath(RsType file_type, bool meta = false);

        boost::filesystem::path parent_, folder_, sub_folder_;
    private:
        const void UpdateTimestamp();
        std::string serial_number_, date_, time_;
        const std::string video_frame_ext = ".png", point_cloud_ext = ".ply", metadata_ext = "_meta.csv";
        const std::string depth_ = "depth_16UC1", coloured_depth_ =  "colourised_depth_8UC3", colour_ = "rgb_8UC3";
        const std::string ir = "ir_8UC1", ir_left_ = "ir_left_8UC1", ir_right_ = "ir_right_8UC1", point_cloud_ =  "point_cloud";
        const std::string file_names_[7] = {depth_, coloured_depth_, colour_, ir, ir_left_, ir_right_, point_cloud_};
        const std::string ext_[3] = {video_frame_ext, point_cloud_ext, metadata_ext};
    };
};

#endif // STRAWBERRYDATA_STRAWBERRY_H