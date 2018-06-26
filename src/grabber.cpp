#include <string>

#include <opencv2/opencv.hpp>
#include <librealsense2/rs.hpp>
#include <json.hpp>

#include <Strawberry.hpp>
#include <RealSenseD400.hpp>
#include <MultiCamD400.hpp>
#include <ConfigManager.hpp>

void PrintHelp() {
    std::cout << "Controls: \n\t-save, s (Writes all output to disk)\n\t-laser0, l0 (Turns laser off)\n\t-laser1 <pa" <<
              "ram>, l1 <param> (Turns laser on)\n\t\t-<param> can be min(-3), mid(-2), max(-1) or any float value" <<
              "\n\t-stab, st (Throws away frames for correcting exposure)" << "\n\t-help, h (Displays help)" <<
              "\n\t-quit, q (Quits)" << std::endl;
}

int main(int argc, char *argv[]) try {
    // Set the singleton class up with the config file
    ConfigManager::SetInstance("../config.json");

    // Initialise currently connected cameras and wait until ready
    // Set refresh rate to 20 Hz since frame rate is only 6
    MultiCamD400 cameras(20);
    cameras.Available();

    // Print the system controls
    PrintHelp();

    char input[255];
    bool quit = false;
    do {
        std::cout << "Enter Control: ";
        std::cin.getline(input, 255, '\n');

        // After receiving the command ensure the cameras are available
        cameras.Available();

        // Parse the input into tokens separated by space
        std::string line(input);
        std::istringstream iss(line);
        std::vector<std::string> sym{std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{}};

        if(!sym.empty()) {
            std::string token = sym[0];
            std::string param = sym.size() > 1 ? sym[1] : "";

            // Check against available commands
            if (token == "laser0" || token == "l0") {
                cameras.SetLaser(false);
            } else if (token == "laser1" || token == "l1") {
                float power = -4;
                // Get the next token and use it as a parameter for power


                // Parse words to power levels
                if (!param.empty()) {
                    if (std::all_of(param.begin(), param.end(), ::isalpha)) {
                        power = param == "min" ? -3 : param == "mid" ? -2 : param == "max" ? -1 : -4;
                    } else if (param.find_first_not_of(".0123456789") == std::string::npos) {
                        power = static_cast<float>(std::stod(param));
                    }
                }

                cameras.SetLaser(true, power);
            } else if (token == "save" || token == "s") {
                cameras.SaveFrames();
            } else if(token == "stab" || token == "st") {
                cameras.StabiliseExposure();
            } else if(token == "help" || token == "h") {
                PrintHelp();
            } else if(token == "quit" || token == "q") {
                quit = true;
            }
        } else {
            cameras.SaveFrames();
        }
    } while (!quit);

    std::cout << "Threads terminated" << std::endl;
    return EXIT_SUCCESS;
}
catch (const rs2::error &e) {
    std::cerr << "RealSense error calling " << e.get_failed_function() << "(" << e.get_failed_args() << "):\n    "
              << e.what() << std::endl;
    return EXIT_FAILURE;
}
catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
}