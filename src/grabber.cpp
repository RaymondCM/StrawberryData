#include <string>

#include <opencv2/opencv.hpp>
#include <librealsense2/rs.hpp>

#include <Strawberry.hpp>
#include <RealSenseD400.hpp>

void PrintDeviceInfo(rs2::device &dev);

int main(int argc, char * argv[]) try
{
    // Get the first real sense device
    rs2::context ctx;
    auto list = ctx.query_devices(); // Get a snapshot of currently connected devices

    if (list.size() == 0)
        throw std::runtime_error("No device detected.");

    rs2::device dev = list.front();

    // Initialise the device class
    RealSenseD400 realsense_camera(dev);
    realsense_camera.StabilizeExposure();

    // Start Capture
    realsense_camera.Stream();

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

