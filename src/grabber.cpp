#include <string>
#include <algorithm>

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
              "\n\t-stab, st (Throws away frames for correcting exposure)" << "\n\t-new, n (Creates new dataset)" <<
              "\n\t-help, h (Displays help)" << "\n\t-quit, q (Quits)" << std::endl;
}

std::string GetInput(std::string message = "") {
    if(!message.empty())
        std::cout << message;
    std::string output;
    getline(std::cin, output);
    return output;
}

std::string GetDefaultInput(std::string message = "", std::string default_input = "") {
    std::string output = GetInput(message);
    return output.empty() ? default_input : output;
}

bool GetYesOrNo() {
    std::string yesno_input;
    getline(std::cin, yesno_input);
    std::transform(yesno_input.begin(), yesno_input.end(), yesno_input.begin(), ::tolower);
    if(yesno_input.empty() || yesno_input == "y" || yesno_input == "yes")
        return true;
    return false;
}

void NewDataset(std::string &project_root, std::string &project_name) {
    std::chrono::high_resolution_clock::time_point p = std::chrono::high_resolution_clock::now();
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(p.time_since_epoch());
    std::time_t t = std::chrono::duration_cast<std::chrono::seconds>(ms).count();

    std::stringstream project_name_buf;
    project_name_buf <<  std::put_time(std::localtime(&t), "data-%Y_%m_%d-%H:%M:%S");

    project_name = project_name_buf.str(), project_root = ConfigManager::IGet("save-path-prefix");

    std::cout << "Creating new dataset:" << std::endl;

    project_root = GetDefaultInput("\tEnter Project Root (Enter for default '" + project_root + "'): ",
                                   project_root);

    if(!project_root.empty() && project_root[project_root.length()] != '\\' && project_root[project_root.length()] != '/') {
        std::cout << "\tDid you mean '" << project_root << "/' instead of '" << project_root << "' (y/n): ";
        if(GetYesOrNo())
            project_root += "/";
    }

    project_name = GetDefaultInput("\tEnter Project Name (Enter for default '" + project_name + "'): ",
                                   project_name);

    // If folder doesn't exist for new data set create it
    boost::filesystem::path folder(project_root + project_name + "/");
    if(!boost::filesystem::is_directory(folder) || !boost::filesystem::exists(folder))
        boost::filesystem::create_directories(folder);

    std::cout << "\tWould you like to create meta-data? GPS, crop distance, notes etc. (y/n): ";
    if(GetYesOrNo()) {
        std::ofstream csv;
        csv.open(folder.string() + "capture_meta.csv");
        //TODO: Auto fill these from https://openweathermap.org/
        csv << "Name (First Last)" << ',' << GetInput("\tName (First Last): ") << '\n';
        csv << "Contact Email" << ',' << GetInput("\tContact Email: ") << '\n';
        csv << "Date (DD/MM/YYYY)" << ',' << GetInput("\tDate (DD/MM/YYYY): ") << '\n';
        csv << "Latitude" << ',' << GetInput("\tLatitude: ") << '\n';
        csv << "Longitude" << ',' << GetInput("\tLongitude: ") << '\n';
        csv << "Location Name" << ',' << GetInput("\tLocation Name: ") << '\n';
        csv << "Location Row ID" << ',' << GetInput("\tLocation Row ID (if known): ") << '\n';
        csv << "Capture Distance from Row (cm)" << ',' << GetInput("\tCapture Distance from Row (cm): ") << '\n';
        csv << "Crop Species" << ',' << GetInput("\tCrop Species (if known): ") << '\n';
        csv << "Notes" << ',' << GetInput("\tNotes: ") << '\n';
        csv.close();
    }

    std::cout << std::endl;
    std::cout << "Use the 'get_weather.py' script and manually append the weather data to the meta file." << std::endl;
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

            // Get the next token and use it as a parameter
            std::string param = sym.size() > 1 ? sym[1] : "";

            // Check against available commands
            if(token == "new" || token == "n") {
                std::string project_root, project_name;
                NewDataset(project_root, project_name);
                cameras.UpdateDataConfiguration(project_name, project_root);
            } else if (token == "laser0" || token == "l0") {
                cameras.SetLaser(false);
            } else if (token == "laser1" || token == "l1") {
                float power = -4;
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