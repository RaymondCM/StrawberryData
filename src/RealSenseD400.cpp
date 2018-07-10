#include <ConfigManager.hpp>
#include "RealSenseD400.hpp"

RealSenseD400::RealSenseD400(rs2::device dev) : dev_(dev), depth_sensor_(dev.first<rs2::depth_sensor>()),
                                                depth_(nullptr), colour_(nullptr),
                                                lir_(nullptr),
                                                rir_(nullptr), c_depth_(nullptr),
                                                data_structure_(dev),
                                                advanced_dev_(dev){
    // Check device is in advanced mode before trying to enable all streams
    // Will cause a could not enable all streams error
    if(!DeviceInAdvancedMode()) {
        std::cout << "Device " << serial_number_ << ": Not in advanced mode, enabling advanced mode" << std::endl;
        //advanced_dev_ = dev.as<rs400::advanced_mode>();
        advanced_dev_.toggle_advanced_mode(true);

        //TODO: Change workflow so that it will reconnect in order to the camera with the same serial
        throw rs2::error("Device advanced mode enabled, device will disconnect and reconnect");
    }
    
    // Print the device information
    PrintDeviceInfo();

    // Get resolution etc from config file
    ConfigManager *config = ConfigManager::GetInstance();
    nlohmann::json depth_config = config->Get("stream-depth");
    nlohmann::json colour_config = config->Get("stream-colour");

    int d_width = depth_config["width"], c_width = colour_config["width"];
    int d_height = depth_config["height"], c_height = colour_config["height"];
    int d_fps = depth_config["frame-rate"], c_fps = colour_config["frame-rate"];

    // Enable IR, depth and colour_ streams at the highest quality streams
    cfg.enable_stream(RS2_STREAM_INFRARED, 1, d_width, d_height, RS2_FORMAT_Y8, d_fps); // Left IR (Colour registered)
    cfg.enable_stream(RS2_STREAM_INFRARED, 2, d_width, d_height, RS2_FORMAT_Y8, d_fps); // Right IR
    cfg.enable_stream(RS2_STREAM_DEPTH, d_width, d_height, RS2_FORMAT_Z16, d_fps);

    // Read in BGR so OpenCV automatically displays/saves it as RGB
    cfg.enable_stream(RS2_STREAM_COLOR, c_width, c_height, RS2_FORMAT_BGR8, c_fps);
    serial_number_ = std::string(dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER));
    cfg.enable_device(serial_number_);

    // Define pipeline with parameters above
    selection = pipe_.start(cfg);

    // Get depth scale (device specific)
    depth_sensor_scale_ = depth_sensor_.get_depth_scale();

    gui_enabled_ = config->Get("gui-enabled");

    // Throwaway some frames to stabilise the exposure
    if (config->Get("stabilise-exposure"))
        StabiliseExposure(config->Get("stabilise-exposure-count"));

    // Update save path
    data_structure_.UpdatePathPrefix(config->Get("save-path-prefix"));

    Setup();
}

RealSenseD400::~RealSenseD400() {
    CloseGUI();
    pipe_.stop();
}

void RealSenseD400::StabiliseExposure(int stabilization_window) {
    // Allow auto exposure to stabilize
    std::cout << "Camera " << serial_number_ << ": Dropping " << stabilization_window << " frames" << std::endl;
    for (int i = 0; i < stabilization_window; i++)
        rs2::frameset exposure_frames = pipe_.wait_for_frames();
}

void RealSenseD400::PrintDeviceInfo() {
    // Check available device information and print it to console
    std::cout << "Device Information: " << std::endl;
    for (int i = 0; i < RS2_CAMERA_INFO_COUNT; ++i) {
        std::string output = std::string(rs2_camera_info_to_string((rs2_camera_info) i)) + ": ";

        try {
            output += std::string(dev_.get_info((rs2_camera_info) i));
        } catch (const rs2::error &) {
            output += "Not available";
        }

        std::cout << "\t" << output << std::endl;
    }
}

bool RealSenseD400::DeviceInAdvancedMode() {
    //    if(dev_.supports(RS2_CAMERA_INFO_ADVANCED_MODE)) {
    //        return dev_.get_info(RS2_CAMERA_INFO_ADVANCED_MODE) == "YES";
    //    }
    //
    //    return false;
    return advanced_dev_.is_enabled();
}

bool RealSenseD400::WindowsAreOpen() {
    // Attempt to decipher whether the OpenCV windows are open
    return cvGetWindowHandle(win_colour_.c_str()) &&
           cvGetWindowHandle(win_ir_.c_str()) &&
           cvGetWindowHandle(win_depth_.c_str());
}

void RealSenseD400::Visualise() {
    if (gui_enabled_) {
        // Concatenate side by side for rendering
        depth_mat_.convertTo(depth_mat_8bit, CV_8UC1, 1.0 / 256.0);
        cv::cvtColor(depth_mat_8bit, depth_mat_8bit, cv::COLOR_GRAY2BGR);
        cv::hconcat(lir_mat_, rir_mat_, lrir_mat);
        cv::hconcat(depth_mat_8bit, c_depth_mat_, cd_depth_mat);

        cv::imshow(win_colour_, colour_mat_);
        cv::imshow(win_depth_, cd_depth_mat);
        cv::imshow(win_ir_, lrir_mat);

        // Get input_
        input_ = static_cast<char>(std::tolower(cv::waitKey(1)));
    }
}

void RealSenseD400::WriteData() {
    //Update folder structure and create necessary folders
    data_structure_.UpdateFolderPaths();

    try {
        // Save the files to disk
        std::cout << "Camera " << serial_number_ << ": Writing " << data_structure_.sub_folder_.string() << std::endl;
        cv::imwrite(data_structure_.FilePath(RsType::DEPTH), depth_mat_);
        cv::imwrite(data_structure_.FilePath(RsType::COLOURED_DEPTH), c_depth_mat_);
        cv::imwrite(data_structure_.FilePath(RsType::COLOUR), colour_mat_);
        cv::imwrite(data_structure_.FilePath(RsType::IR_LEFT), lir_mat_);
        cv::imwrite(data_structure_.FilePath(RsType::IR_RIGHT), rir_mat_);
        point_cloud_.export_to_ply(data_structure_.FilePath(RsType::POINT_CLOUD), colour_);

        // Write meta data
        WriteVideoFrameMetaData(data_structure_.FilePath(RsType::DEPTH, true), depth_);
        WriteVideoFrameMetaData(data_structure_.FilePath(RsType::COLOUR, true), colour_);
        WriteVideoFrameMetaData(data_structure_.FilePath(RsType::IR, true), lir_);
    }
    catch (const rs2::error &e) {
        std::cerr << "RealSense error calling " << e.get_failed_function() << "(" << e.get_failed_args() << "):\n    "
                  << e.what() << std::endl;
    }
    catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}


void RealSenseD400::WriteVideoFrameMetaData(const std::string &file_name, rs2::video_frame &frame) {
    std::ofstream csv;
    csv.open(file_name);

    //std::cout << "Writing metadata to " << file_name << std::endl;
    csv << "Stream," << rs2_stream_to_string(frame.get_profile().stream_type()) << "\nMetadata Attribute,Value\n";

    // Record all the available metadata attributes
    for (size_t i = 0; i < RS2_FRAME_METADATA_COUNT; i++) {
        if (frame.supports_frame_metadata((rs2_frame_metadata_value) i)) {
            csv << rs2_frame_metadata_to_string((rs2_frame_metadata_value) i) << "," <<
                frame.get_frame_metadata((rs2_frame_metadata_value) i) << "\n";
        }
    }

    csv.close();
}

void RealSenseD400::WriteDeviceData(const std::string &file_name) {
    std::ofstream csv;
    csv.open(file_name);

    std::cout << "Camera " << serial_number_ << ": Writing device data " << file_name << std::endl;

    // Camera Info
    for (int i = 0; i < RS2_CAMERA_INFO_COUNT; ++i)
        if (dev_.supports((rs2_camera_info) i))
            csv << rs2_camera_info_to_string((rs2_camera_info) i) << "," << dev_.get_info((rs2_camera_info) i) << '\n';

    // Depth Options
    for (int i = 0; i < RS2_OPTION_COUNT; ++i)
        if (depth_sensor_.supports((rs2_option) i))
            csv << rs2_option_to_string((rs2_option) i) << "," << depth_sensor_.get_option((rs2_option) i) << '\n';

    csv.close();
}

const void RealSenseD400::Setup() {
    if (gui_enabled_) {
        win_colour_ += " " + serial_number_;
        win_ir_ += " " + serial_number_;
        win_depth_ += " " + serial_number_;

        // Create preview GUI
        //cv::namedWindow(win_colour_, cv::WINDOW_GUI_EXPANDED | CV_WINDOW_NORMAL | CV_GUI_NORMAL);
        //cv::namedWindow(win_ir_, cv::WINDOW_GUI_EXPANDED | CV_WINDOW_NORMAL | CV_GUI_NORMAL);
        //cv::namedWindow(win_depth_, cv::WINDOW_GUI_EXPANDED | CV_WINDOW_NORMAL | CV_GUI_NORMAL);
    }

    //Update folder structure and create necessary folders
    data_structure_.UpdateFolderPaths(true);
    WriteDeviceData(data_structure_.folder_.string() + serial_number_ + "_meta.csv");
}

const void RealSenseD400::WaitForFrames() {
    //std::cout << "Camera " << serial_number_ << " waiting for frames" << std::endl;

    int attempts = 0;
    do {
        try {
            if (attempts > 0)
                std::cerr << "\nCamera " << serial_number_ << ": Invalid frame, waiting for next coherent set (Attempt"
                          << attempts << ")" << std::endl;

            // Wait for a coherent set of frames
            //if(pipe_.poll_for_frames(&frames_)) {
            frames_ = pipe_.wait_for_frames();
            depth_ = frames_.get_depth_frame();
            colour_ = frames_.get_color_frame();
            lir_ = frames_.get_infrared_frame(1);
            rir_ = frames_.get_infrared_frame(2);
            c_depth_ = color_map(depth_);

            // Map to depth_ frame
            pc_.map_to(depth_);
            point_cloud_ = pc_.calculate(depth_);

            // Validate the frames
            attempts++;
            //}
        } catch (rs2::error &e) {
            std::cerr << "Camera " << serial_number_ << ": " << e.what() << std::endl;
            attempts++;
        }
    } while (!colour_ || !depth_ || !c_depth_ || !lir_ || !rir_ || !point_cloud_);


    // Create OpenCV objects
    colour_mat_ = cv::Mat(cv::Size(colour_.get_width(), colour_.get_height()), CV_8UC3, (void *) colour_.get_data());
    depth_mat_ = cv::Mat(cv::Size(depth_.get_width(), depth_.get_height()), CV_16UC1, (void *) depth_.get_data());
    c_depth_mat_ = cv::Mat(cv::Size(c_depth_.get_width(), c_depth_.get_height()), CV_8UC3,
                           (void *) c_depth_.get_data());
    lir_mat_ = cv::Mat(cv::Size(lir_.get_width(), lir_.get_height()), CV_8UC1, (void *) lir_.get_data());
    rir_mat_ = cv::Mat(cv::Size(rir_.get_width(), rir_.get_height()), CV_8UC1, (void *) rir_.get_data());

    frame_id_++;

    // Show the output
    Visualise();
}


const void RealSenseD400::SetLaser(bool status, float power) {
    std::cout << "Camera " << serial_number_ << ": Setting laser to " << (status ? "on" : "off");

    if (depth_sensor_.supports(RS2_OPTION_EMITTER_ENABLED))
        depth_sensor_.set_option(RS2_OPTION_EMITTER_ENABLED, status);

    if (status && power != -4 && depth_sensor_.supports(RS2_OPTION_LASER_POWER)) {
        // Query min and max values:
        auto range = depth_sensor_.get_option_range(RS2_OPTION_LASER_POWER);

        // Special values for -1:MAX -2:MID
        if (power == -1)
            power = range.max;
        else if (power == -2)
            power = (range.min + range.max) / 2;
        else if (power == -3)
            power = range.min;

        depth_sensor_.set_option(RS2_OPTION_LASER_POWER, power);
        std::cout << " at " << power << " power" << std::endl;
    } else {
        std::cout << std::endl;
    }
}

rs2::pipeline_profile RealSenseD400::GetProfile() {
    return selection;
}

void RealSenseD400::CloseGUI() {
    if (gui_enabled_) {
        cv::destroyWindow(win_colour_);
        cv::destroyWindow(win_depth_);
        cv::destroyWindow(win_ir_);
    }
}
