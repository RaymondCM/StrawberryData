#ifndef STRAWBERRYDATA_MULTICAMD400_H
#define STRAWBERRYDATA_MULTICAMD400_H

#include "ThreadClass.hpp"
#include "RealSenseD400.hpp"
#include <librealsense2/rs.hpp>

class MultiCamD400 : ThreadClass {
public:
    explicit MultiCamD400(unsigned int hz=60);
    const void AddDevice(rs2::device dev);
    const void RemoveDevice(const rs2::event_information& info);
    const void SaveFrames();
    const void SaveFrames(int index);
    const void SetLaser(bool laser, float power=-4);
    const void SetLaser(int index, bool laser, float power=-4);
    const void StabiliseExposure();
    const void StabiliseExposure(int index);
    const void Pause(bool pause=true);

    // Utility function for calling methods
    void Available();
    bool initialised = false;
private:
    std::map<std::string, RealSenseD400*> cameras_;
    const void Setup() override;
    const void Loop() override;
    bool loop_paused_;

    // Flip guard to toggle between two values on scope/set default value
    template <typename T>
    class flip_guard {
        T * f, fs, fe;
    public:
        flip_guard(T *v) {f = v; fs = *v;}
        flip_guard(T *v, T s) : flip_guard(v) {fs = s; fe = !s; *f = fs;}
        flip_guard(T *v, T s, T e) : flip_guard(v, s) {fe = e;}
        ~flip_guard() {*f = fe;}
    };
};


#endif //STRAWBERRYDATA_MULTICAMD400_H
