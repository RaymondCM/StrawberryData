#include <string>
#include <chrono>
#include <ctime>

#include <boost/filesystem.hpp>

#include <opencv2/opencv.hpp>
#include <librealsense2/rs.hpp>

void PrintDeviceInfo(rs2::device &dev);
void WriteMetaData(const std::string &file_name, rs2::video_frame &frame);

int main(int argc, char * argv[]) try
{
    // Get the first real sense device
    rs2::context ctx;
    auto list = ctx.query_devices(); // Get a snapshot of currently connected devices

    if (list.size() == 0)
        throw std::runtime_error("No device detected.");

    rs2::device dev = list.front();

    // Print the device information
    PrintDeviceInfo(dev);

    // Enable IR, depth and colour streams at the highest quality streams
    rs2::config cfg;
    cfg.enable_stream(RS2_STREAM_INFRARED, 1, 1280, 720, RS2_FORMAT_Y8, 30); // Left IR (Colour registered)
    cfg.enable_stream(RS2_STREAM_INFRARED, 2, 1280, 720, RS2_FORMAT_Y8, 30); // Right IR
    cfg.enable_stream(RS2_STREAM_DEPTH, 1280, 720, RS2_FORMAT_Z16, 30);
    cfg.enable_stream(RS2_STREAM_COLOR, 1920, 1080, RS2_FORMAT_BGR8, 30);
    std::string serial_number(dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER));
    cfg.enable_device(serial_number);

    // Define pipeline with parameters above
    rs2::pipeline pipe;
    auto selection = pipe.start(cfg);

    // Get depth scale (device specific)
    auto sensor = selection.get_device().first<rs2::depth_sensor>();
    auto scale =  sensor.get_depth_scale();

    // Create colourise object for pretification of depth image
    rs2::colorizer color_map;

    // Declare point cloud object for calculating the point cloud and mapping
    rs2::pointcloud pc;

    // Create preview GUI
    const std::string win_colour("Colour"), win_ir("IR (Left, Right)"), win_depth("Depth (Uncoloured, Colourised)");
    cv::namedWindow(win_colour);
    cv::namedWindow(win_ir);
    cv::namedWindow(win_depth);

    // Lambda to check if the windows are still open
    auto windows_are_open = [&win_colour, &win_ir, &win_depth]() {
        return cvGetWindowHandle(win_colour.c_str()) &&
               cvGetWindowHandle(win_ir.c_str()) &&
               cvGetWindowHandle(win_depth.c_str());
    };

    // Allow auto exposure to stabilize
    for(int i = 0; i < 30; i++)
        rs2::frameset exposure_frames = pipe.wait_for_frames();

    char input = 's';
    while(input != 'q' && windows_are_open()) {
        // Wait for a coherent set of frames
        auto frames = pipe.wait_for_frames();
        auto depth = frames.get_depth_frame();
        auto colour = frames.get_color_frame();
        auto lir = frames.get_infrared_frame(1);
        auto rir = frames.get_infrared_frame(2);
        auto c_depth = color_map(depth);

        // Map to colour frame
        pc.map_to(colour);
        auto point_cloud = pc.calculate(depth);

        // Validate the frames
        if(!colour || !depth || !c_depth || !lir || !rir || !point_cloud) {
            std::cerr << "Invalid frame, waiting for next coherent set" << std::endl;
            continue;
        }

        // Create OpenCV objects
        cv::Mat colour_mat = cv::Mat(cv::Size(colour.get_width(), colour.get_height()), CV_8UC3, (void*)colour.get_data());
        cv::Mat depth_mat = cv::Mat(cv::Size(depth.get_width(), depth.get_height()), CV_16UC1, (void*)depth.get_data());
        cv::Mat c_depth_mat = cv::Mat(cv::Size(c_depth.get_width(), c_depth.get_height()), CV_8UC3, (void*)c_depth.get_data());
        cv::Mat lir_mat = cv::Mat(cv::Size(lir.get_width(), lir.get_height()), CV_8UC1, (void*)lir.get_data());
        cv::Mat rir_mat = cv::Mat(cv::Size(rir.get_width(), rir.get_height()), CV_8UC1, (void*)rir.get_data());

        // Concatenate side by side for rendering
        cv::Mat lrir_mat, cd_depth_mat, depth_mat_8bit;
        depth_mat.convertTo(depth_mat_8bit, CV_8UC1, 1.0 / 256.0);
        cv::cvtColor(depth_mat_8bit, depth_mat_8bit, cv::COLOR_GRAY2BGR);
        cv::hconcat(lir_mat, rir_mat, lrir_mat);
        cv::hconcat(depth_mat_8bit, c_depth_mat, cd_depth_mat);

        cv::imshow(win_colour, colour_mat);
        cv::imshow(win_depth, cd_depth_mat);
        cv::imshow(win_ir, lrir_mat);

        // Get input
        input = static_cast<char>(std::tolower(cv::waitKey(1)));

        // If the input is a space then save all of the data
        if(input == ' ') {
            // Create folder name data/YYYY_MM_DD/HH_MM_SS_MMMM/file
            using namespace std::chrono;
            high_resolution_clock::time_point p = high_resolution_clock::now();
            milliseconds ms = duration_cast<milliseconds>(p.time_since_epoch());
            std::time_t t = duration_cast<seconds>(ms).count();
            long fractional_seconds = ms.count() % 1000;
            std::stringstream folder;
            folder << "data/" << serial_number << std::put_time(std::localtime(&t), "/%Y_%m_%d/%H_%M_%S_") <<
                   std::setfill('0') << std::setw(3) << fractional_seconds << "/";
            std::string folder_path = folder.str();
            boost::filesystem::path dir(folder_path);

            // Create directories if they do not exist
            if(!boost::filesystem::is_directory(dir) || !boost::filesystem::exists(dir))
                boost::filesystem::create_directories(dir);

            // Save the files to disk
            std::cout << "Writing files " << folder_path << std::endl;
            cv::imwrite(folder_path + "depth_16UC1.png", depth_mat);
            cv::imwrite(folder_path + "colourised_depth_8UC1.png", c_depth_mat);
            cv::imwrite(folder_path + "bgr_8UC3.png", colour_mat);
            cv::imwrite(folder_path + "ir_left_8UC1.png", lir_mat);
            cv::imwrite(folder_path + "ir_right_8UC1.png", rir_mat);
            point_cloud.export_to_ply(folder_path + "point_cloud.ply", colour);

            // Write meta data
            WriteMetaData(folder_path + "bgr_8UC3_metadata.csv", colour);
            WriteMetaData(folder_path + "depth_16UC1_metadata.csv", depth);
            WriteMetaData(folder_path + "ir_8UC1_metadata.csv", lir);
        }
    }

    pipe.stop();
    return EXIT_SUCCESS;
}
catch (const rs2::error & e)
{
    std::cerr << "RealSense error calling " << e.get_failed_function() << "(" << e.get_failed_args() << "):\n    " << e.what() << std::endl;
    return EXIT_FAILURE;
}
catch (const std::exception & e)
{
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
}

void PrintDeviceInfo(rs2::device &dev) {
    auto print_info = [&dev](const int info) {
        std::string output = "Not available";
        try {
            output = std::string(dev.get_info((rs2_camera_info) info));
        } catch (const rs2::error&) {}
        return std::string(rs2_camera_info_to_string((rs2_camera_info) info)) + ": " + output;
    };

    std::cout << "Device Information: " << std::endl;
    for (int i = 0; i < RS2_CAMERA_INFO_COUNT; ++i)
        std::cout << "\t" << print_info(i) << std::endl;
}

void WriteMetaData(const std::string &file_name, rs2::video_frame &frame) {
    std::ofstream csv;
    csv.open(file_name);

    //std::cout << "Writing metadata to " << file_name << std::endl;
    csv << "Stream," << rs2_stream_to_string(frame.get_profile().stream_type()) << "\nMetadata Attribute,Value\n";

    // Record all the available metadata attributes
    for (size_t i = 0; i < RS2_FRAME_METADATA_COUNT; i++)
    {
        if (frame.supports_frame_metadata((rs2_frame_metadata_value)i))
        {
            csv << rs2_frame_metadata_to_string((rs2_frame_metadata_value)i) << "," <<
                frame.get_frame_metadata((rs2_frame_metadata_value)i) << "\n";
        }
    }

    csv.close();
}
