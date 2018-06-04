#include "RealSenseD400.hpp"

RealSenseD400::RealSenseD400(rs2::device dev, bool gui) : dev_(dev), depth_sensor_(dev.first<rs2::depth_sensor>()),
                                                          depth_(nullptr), colour_(nullptr), lir_(nullptr),
                                                          rir_(nullptr), c_depth_(nullptr), data_structure_(dev),
                                                          ThreadClass(60)  {
    // Print the device information
    PrintDeviceInfo();

    // Enable IR, depth and colour_ streams at the highest quality streams
    cfg.enable_stream(RS2_STREAM_INFRARED, 1, 1280, 720, RS2_FORMAT_Y8, 30); // Left IR (Colour registered)
    cfg.enable_stream(RS2_STREAM_INFRARED, 2, 1280, 720, RS2_FORMAT_Y8, 30); // Right IR
    cfg.enable_stream(RS2_STREAM_DEPTH, 1280, 720, RS2_FORMAT_Z16, 30);
    cfg.enable_stream(RS2_STREAM_COLOR, 1920, 1080, RS2_FORMAT_BGR8, 30);
    serial_number_ = std::string(dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER));
    cfg.enable_device(serial_number_);

    // Define pipeline with parameters above
    selection = pipe_.start(cfg);

    // Get depth scale (device specific)
    depth_sensor_scale_ =  depth_sensor_.get_depth_scale();

    gui_enabled_ = gui;

    StartThread();
}

RealSenseD400::~RealSenseD400() {
    pipe_.stop();
}

void RealSenseD400::StabilizeExposure(int stabilization_window) {
    // Allow auto exposure to stabilize
    for(int i = 0; i < 30; i++)
        rs2::frameset exposure_frames = pipe_.wait_for_frames();
}

void RealSenseD400::PrintDeviceInfo() {
    std::cout << "Device Information: " << std::endl;
    for (int i = 0; i < RS2_CAMERA_INFO_COUNT; ++i) {
        std::string output = std::string(rs2_camera_info_to_string((rs2_camera_info) i)) + ": ";

        try {
            output += std::string(dev_.get_info((rs2_camera_info) i));
        } catch (const rs2::error&) {
            output += "Not available";
        }

        std::cout << "\t" << output << std::endl;
    }
}

bool RealSenseD400::WindowsAreOpen() {
    return cvGetWindowHandle(win_colour_.c_str()) &&
           cvGetWindowHandle(win_ir_.c_str()) &&
           cvGetWindowHandle(win_depth_.c_str());
}

void RealSenseD400::Visualise() {
    if(gui_enabled_) {
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
    // Lock prevents data being overwritten before written to disk
    // delays further capture until written (cheap camera sync)
    lock_mutex_.lock();

    //Update folder structure and create necessary folders
    data_structure_.UpdateFolderPaths();

    // Save the files to disk
    std::cout << "Writing files " << data_structure_.sub_folder_.string() << std::endl;
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

    lock_mutex_.unlock();
}


void RealSenseD400::WriteVideoFrameMetaData(const std::string &file_name, rs2::video_frame &frame) {
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

const void RealSenseD400::Setup() {
    if(gui_enabled_) {
        win_colour_ += " " + serial_number_;
        win_ir_ += " " + serial_number_;
        win_depth_ += " " + serial_number_;

        // Create preview GUI
        cv::namedWindow(win_colour_);
        cv::namedWindow(win_ir_);
        cv::namedWindow(win_depth_);
    }

    StabilizeExposure();

    Loop();
}

const void RealSenseD400::Loop() {
    while(ThreadAlive() && input_ != 'q') {
        // Wait for a coherent set of frames
        frames_ = pipe_.wait_for_frames();

        // When coherent frames arrive lock mutex to disable read/write to frame data
        // also waits to override frame data if mutex is already locked
        lock_mutex_.lock();

        depth_ = frames_.get_depth_frame();
        colour_ = frames_.get_color_frame();
        lir_ = frames_.get_infrared_frame(1);
        rir_ = frames_.get_infrared_frame(2);
        c_depth_ = color_map(depth_);

        // Map to colour_ frame
        pc_.map_to(colour_);
        point_cloud_ = pc_.calculate(depth_);

        // Validate the frames
        if(!colour_ || !depth_ || !c_depth_ || !lir_ || !rir_ || !point_cloud_) {
            std::cerr << "Invalid frame, waiting for next coherent set" << std::endl;
            lock_mutex_.unlock();
            continue;
        }

        // Create OpenCV objects
        colour_mat_ = cv::Mat(cv::Size(colour_.get_width(), colour_.get_height()), CV_8UC3, (void*)colour_.get_data());
        depth_mat_ = cv::Mat(cv::Size(depth_.get_width(), depth_.get_height()), CV_16UC1, (void*)depth_.get_data());
        c_depth_mat_ = cv::Mat(cv::Size(c_depth_.get_width(), c_depth_.get_height()), CV_8UC3, (void*)c_depth_.get_data());
        lir_mat_ = cv::Mat(cv::Size(lir_.get_width(), lir_.get_height()), CV_8UC1, (void*)lir_.get_data());
        rir_mat_ = cv::Mat(cv::Size(rir_.get_width(), rir_.get_height()), CV_8UC1, (void*)rir_.get_data());

        frame_id_++;
        lock_mutex_.unlock();

        // Show the output
        Visualise();
    }

    if(gui_enabled_) {
        cv::destroyWindow(win_colour_);
        cv::destroyWindow(win_depth_);
        cv::destroyWindow(win_ir_);
    }

    cancel_thread_ = true;
}


const void RealSenseD400::SetLaser(bool status, float power) {
    if (depth_sensor_.supports(RS2_OPTION_EMITTER_ENABLED))
        depth_sensor_.set_option(RS2_OPTION_EMITTER_ENABLED, status);

    if (status && power != -4 && depth_sensor_.supports(RS2_OPTION_LASER_POWER))
    {
        // Query min and max values:
        auto range = depth_sensor_.get_option_range(RS2_OPTION_LASER_POWER);

        // Special values for -1:MAX -2:MID
        if(power == -1)
            power = range.max;
        else if (power == -2)
            power = (range.min + range.max) / 2;
        else if (power == -3)
            power = range.min;

        depth_sensor_.set_option(RS2_OPTION_LASER_POWER, power);
    }
}