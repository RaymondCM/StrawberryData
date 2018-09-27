#ifndef STRAWBERRYDATA_DATASETPARSER_H
#define STRAWBERRYDATA_DATASETPARSER_H

#include <chrono>
#include <string>
#include <vector>
#include <map>

namespace DatasetParser {
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
        CaptureImage(std::string folder_path);
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
        RGBCapture(std::string folder_root);

    };

    class DepthCapture : CaptureImage {
    public:
        DepthCapture(std::string folder_root);
        std::string GetDepthPath();
        std::string GetColourisedDepthPath();

    private:
        std::string colourised_depth_absolute_path_;
    };

    class IRCapture : CaptureImage {
    public:
        IRCapture(std::string folder_root);
        std::string GetLeftIRPath();
        std::string GetRightIRPath();

    private:
        std::string right_ir_absolute_path_;
    };

    enum class SensorType : int8_t {
        RGB = 0,
        IR_LEFT = 1,
        IR_RIGHT = 2,
        DEPTH = 3,
        COLOURISED_DEPTH = 4
    };

    class DataCapture {
    public:
        DataCapture();
        RGBCapture rgb;
        DepthCapture depth;
        IRCapture ir;
        std::string GetPath(SensorType sensor_type);
        std::string GetRGBPath();
        std::string GetDepthPath(SensorType sensor_type = SensorType::DEPTH);
        std::string GetIRPath(SensorType sensor_type = SensorType::IR_LEFT);
    };

    class CameraMeta {
    public:
        std::string name, serial_number;
    private:
        std::string path;
    };

    class CameraData {
    public:
        CameraMeta camera_meta;
        std::map<timestamp, std::vector<DataCapture>> data;
    private:
        std::string camera_folder_path;
    };

    class SessionMeta


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
};


#endif //STRAWBERRYDATA_DATASETPARSER_H
