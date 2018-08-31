#include <string>
#include <algorithm>

#include <opencv2/opencv.hpp>
#include <librealsense2/rs.hpp>
#include <json.hpp>

#include <Strawberry.hpp>
#include <RealSenseD400.hpp>
#include <MultiCamD400.hpp>
#include <ConfigManager.hpp>

int main(int argc, char *argv[]) try {
    // Set the singleton class up with the config file
    ConfigManager::SetInstance("../config.json");

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

using timestamp = std::chrono::high_resolution_clock::time_point;

class CaptureMeta {
public:
    CaptureMeta(std::string path);
    timestamp frame, sensor, arrival, backend;
    int frame_counter_id;
private:
    std::string meta_path;
};


class CaptureImage {
public:
    CaptureImage(std::string folder_path, std::string meta_name);
    CaptureImage(const CaptureImage&) = delete;
    void operator=(const CaptureImage &capture_image) = delete;
    bool LoadCaptureMeta();
    virtual std::string GetPath();
    timestamp GetTimestamp();
    timestamp GetFrameTimestamp();
    timestamp GetSensorTimestamp();
    timestamp GetArrivalTimestamp();
    timestamp GetBackendTimestamp();
private:
    std::string absolute_path;
    CaptureMeta capture_meta;
};

class RGBCapture : CaptureImage {
public:
    RGBCapture(std::string folder_root, std::string rgb_meta) : CaptureImage(folder_root, rgb_meta) {};

};



// Each image: (RGB, depth and IR)
// class capture_meta
//  public
//      time frame
//      time sensor
//      time arrival
//      time back end
//      int frame_counter_id

// abstract class capture_image (RGB, depth and Ir images)
//  private
//      grabber_type RGB | IR | Depth
//      string absolute path
//      capture_meta meta_data
//  public
//      get path
//      get frame time
//      get sensor time
//      get arrival time
//      get backend time
//      get time

// class rgb_capture : capture_image

// class depth_capture : capture_image
//  private
//      std::string depth_absolute_path
//      std::string colourised_depth_absolute_path
//  public
//      get_path -> get depth path
//      get depth path
//      get colourised depth path

// class ir_capture : capture_image
//  private
//      std::string ir_left_absolute_path
//      std::string ir_right_absolute_path
//  public
//      get_path -> get left path
//      get left path
//      get right path

// enum SENSOR_TYPE {
//  RGB = 0
//  IR_LEFT = 1
//  IR_RIGHT = 2
//  DEPTH = 3
//  COLOURISED_DEPTH = 4
// }

// class data_capture Each capture image set is part of a single grabber capture
// public
//      rgb_capture
//      ir_capture
//      depth_capture
//      get path <- SENSOR_TYPE
//      get rgb path
//      get ir path <- SENSOR_TYPE : default 0
//      get depth path <- SENSOR_TYPE : default 0

/*
 * class camera_meta
 *  public
 *      string path
 *      string name
 *      string serial_number
 */

/*
 * class camera_data
 *  private
*       string sessions_folder_path (where all the timestamp folders are)
 *  public
 *      camera_meta
 *      std::map<session_date, std::vector<data_capture>> data
 */

/*
 * class session_meta
 *  public
 *      string name
 *      string contact
 *      string lat
 *      string lon
 *      string location_name
 *      string capture_distance
 *      string crop
 *      string notes
 *      int clouds
 *      string detailed_weather_status
 *      int humidity
 *      int pressure
 *      int sea_level_pressure
 *      int last_3h_rain
 *      time reference
 *      time sunrise
 *      time sunset
 *      float kelvin_temp
 *      int visibility
 *      int weather code
 *      string country code
 *      time weather_info_reception
 *      string weather_loc_id
 *      string weather lat
 *      string weather lon
 *      string weather loc name
 *      string weather status
 *      int wind degree
 *      int wind speed
 *
 */


/*
 * class session_data
 *  private
 *      string session_path (where the cameras are 3 folders)
 *  public
 *      std::vector<camera_data>
 *      session_meta
 */

/*
 * class grabber_data
 *  private
 *      string data_folder //TopLevel data folder
 *  public
 *      session_data
 */

/*
 * class grabber_dataset
 *  private
 *      std::vector<grabber_data> // Support for multiple data roots (start with one)
 *  public
 *    void GetByHierarchy()
 *    void GetByDay()
 *    void GetByImage()
 *    void GetByWhyCON()
 *    void GetByStartTime()
 *    void GetByDataSet()
 *    void GetByWeather()
 *    void GetByWeather()
*/