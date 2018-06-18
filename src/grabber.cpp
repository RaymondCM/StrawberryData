#include <string>

#include <opencv2/opencv.hpp>
#include <librealsense2/rs.hpp>

#include <Strawberry.hpp>
#include <RealSenseD400.hpp>
#include <MultiCamD400.hpp>

void PrintHelp() {
    std::cout << "Controls: \n\t-save, s (Writes all output to disk)\n\t-laser0, l0 (Turns laser off)\n\t-laser1 <pa" <<
              "ram>, l1 <param> (Turns laser on)\n\t\t-<param> can be min(-3), mid(-2), max(-1) or any float value" <<
              "\n\t-help, h (Displays help)" << "\n\t-quit, q (Quits the application)" << std::endl;
}

int main(int argc, char *argv[]) try {
    MultiCamD400 cameras;
    cameras.Available();

    PrintHelp();

    char input[255];
    bool quit = false;
    do {
        std::cout << "Enter Control: ";
        cameras.Available();

        std::cin.getline(input, 255, '\n');

        std::string line(input);
        std::istringstream iss(line);
        std::vector<std::string> sym{std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{}};

        for (int i = 0; i < sym.size(); i++) {
            std::string token = sym[i];

            if (token == "laser0" || token == "l0") {
                cameras.SetLaser(false);
            } else if (token == "laser1" || token == "l1") {
                float power = -4;
                std::string param = i < sym.size() - 1 ? sym[i + 1] : "";

                if (!param.empty())
                    if (std::all_of(param.begin(), param.end(), ::isalpha))
                        power = param == "min" ? -3 : param == "mid" ? -2 : param == "max" ? -1 : -4;
                    else if (param.find_first_not_of(".0123456789") == std::string::npos)
                        power = static_cast<float>(std::stod(param));

                cameras.SetLaser(true, power);
            } else if (token == "save" || token == "s") {
                cameras.SaveFrames();
            } else if(token == "help" || token == "h") {
                PrintHelp();
            } else if(token == "quit" || token == "q") {
                quit = true;
            }
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