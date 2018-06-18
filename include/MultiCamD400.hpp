#ifndef STRAWBERRYDATA_MULTICAMD400_H
#define STRAWBERRYDATA_MULTICAMD400_H

#include "ThreadClass.hpp"
#include "RealSenseD400.hpp"
#include <librealsense2/rs.hpp>

class MultiCamD400 : ThreadClass {
public:
    explicit MultiCamD400(bool wait_for_stabilise_exposure=false);
    const void AddDevice(rs2::device dev);
    const void RemoveDevice(const rs2::event_information& info);
    const void SaveFrames();
    const void SaveFrames(int index);
    const void SetLaser(bool laser, float power=-4);
    const void SetLaser(int index, bool laser, float power=-4);
    const void StabiliseExposure();
    const void StabiliseExposure(int index);

    //Utility function for calling methods
    void Available();
    bool initialised = false;
private:
    std::map<std::string, RealSenseD400*> cameras_;
    const void Setup() override;
    const void Loop() override;
    bool should_stabilise_exposure;
};


#endif //STRAWBERRYDATA_MULTICAMD400_H
