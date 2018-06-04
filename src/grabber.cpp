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

    std::vector<rs2::device> devices;
    std::vector<RealSenseD400*> cameras;

    // Initialise the devices
    // Wait for capture to end
    for (int i = 0; i < list.size(); ++i) {
        devices.push_back(list[0]);
        cameras.push_back(new RealSenseD400(list[0]));
    }

    bool all_threads_alive = true;
    char input[255];

    do {
        std::cout << "Press enter to save data from all capture devices: ";
        std::cin.getline(input, 255, '\n');

        for(auto & cam : cameras)
            all_threads_alive &= cam->ThreadAlive();

        if(all_threads_alive) {
            std::string line(input);
            std::istringstream iss(line);
            std::vector<std::string> sym{std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{}};

            for(int i = 0; i < sym.size(); i++) {
                std::string token = sym[i];

                if(token == "laser0" || token == "l0") {
                    for(auto & cam : cameras)
                        cam->SetLaser(false);
                } else if(token == "laser1" || token == "l1") {
                    float power = -4;
                    std::string param = i < sym.size() - 1 ? sym[i+1] : "";

                    if(!param.empty())
                        if(std::all_of(param.begin(), param.end(), ::isalpha))
                            power = param == "min" ? -3 : param == "mid" ? -2 : param == "max" ? -1 : -4;
                        else if(param.find_first_not_of(".0123456789") == std::string::npos)
                            power = static_cast<float>(std::stod(param));

                    for(auto & cam : cameras)
                        cam->SetLaser(true, power);
                } else if(token == "save" || token == "s") {
                    for(auto & cam : cameras)
                        cam->WriteData();
                }
            }

        }
    } while(all_threads_alive);

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

