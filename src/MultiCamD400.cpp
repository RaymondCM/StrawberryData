#include "MultiCamD400.hpp"

MultiCamD400::MultiCamD400(unsigned int hz) : ThreadClass(hz) {
    StartThread();
}

const void MultiCamD400::Setup() {
    // Get the first real sense device
    rs2::context ctx;

    // When devices are changed update connected devices
    ctx.set_devices_changed_callback([&](rs2::event_information &info) {
        initialised = false;
        RemoveDevice(info);
        for (auto &&dev : info.get_new_devices())
            AddDevice(dev);
        initialised = true;
    });

    // Get the list of currently connected devices
    auto list = ctx.query_devices();

    if (list.size() == 0) {
        //throw std::runtime_error("No device detected.");
        std::cerr << "No devices connected in at run time, verify with 'lsusb'" << std::endl;
    }

    // Initialise the devices
    for (auto &&cam : list)
        AddDevice(cam);

    initialised = true;

    while (ThreadAlive()) {
        try {
            //Wrap the loop logic around two time points
            start_ = std::chrono::system_clock::now();
            Loop();
            end_ = std::chrono::system_clock::now();

            //Calculate how long the thread should sleep for
            // e.g. if Loop() took 4ms and freq is 100Hz then only sleep for 6ms instead of 10ms
            sleep_for_ = ms_timeout_ - std::chrono::duration_cast<std::chrono::milliseconds>(end_ - start_).count();

            //Refresh Every N(Hz) by calculating timeout in ms - time took to execute (if < 0 then = 0)
            if (sleep_for_ >= 0)
                std::this_thread::sleep_for(std::chrono::milliseconds(sleep_for_));
        } catch (const std::exception &err) {
            std::cerr << "Error: " << err.what() << std::endl;
            cancel_thread_ = true;
        }
    }
}

const void MultiCamD400::Loop() {
    if (!loop_paused_) {
        std::lock_guard<std::mutex> lock(lock_mutex_);
        for (auto &&cam : cameras_)
            cam.second->WaitForFrames();
    }
}

const void MultiCamD400::AddDevice(rs2::device dev) {
    flip_guard<bool> pause(&loop_paused_, true);
    std::lock_guard<std::mutex> lock(lock_mutex_);

    std::string serial_number(dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER));

    // Return if the device is already connected
    if (cameras_.find(serial_number) != cameras_.end())
        return;

    try {
        cameras_.emplace(serial_number, new RealSenseD400(dev));
    } catch (rs2::error &e) {
        std::cerr << e.what() << std::endl;
    }
}

const void MultiCamD400::RemoveDevice(const rs2::event_information &info) {
    flip_guard<bool> pause(&loop_paused_, true);
    std::lock_guard<std::mutex> lock(lock_mutex_);

    // Go over the list of devices and check if it was disconnected if so remove it
    auto itr = cameras_.begin();
    while (itr != cameras_.end())
        if (info.was_removed(itr->second->GetProfile().get_device())) {
            itr->second->CloseGUI();
            itr = cameras_.erase(itr);
        } else
            ++itr;
}

const void MultiCamD400::SaveFrames() {
    flip_guard<bool> pause(&loop_paused_, true);
    std::lock_guard<std::mutex> lock(lock_mutex_);

    if(cameras_.empty()) {
        std::cerr << "No devices connected, restart application or verify with 'lsusb'" << std::endl;
        return;
    }

    std::vector<std::thread> threads;
    for (auto &&cam : cameras_)
        threads.emplace_back(std::bind([&cam]() { cam.second->WriteData(); }));
    std::for_each(threads.begin(), threads.end(), [](std::thread &t) { t.join(); });
}

const void MultiCamD400::SaveFrames(int index) {
    flip_guard<bool> pause(&loop_paused_, true);
    std::lock_guard<std::mutex> lock(lock_mutex_);

    if(cameras_.empty()) {
        std::cerr << "No devices connected, restart application or verify with 'lsusb'" << std::endl;
        return;
    }

    int i = 0;
    for (auto &&cam : cameras_)
        if (index == i++)
            cam.second->WriteData();
}

const void MultiCamD400::SetLaser(bool laser, float power) {
    flip_guard<bool> pause(&loop_paused_, true);
    std::lock_guard<std::mutex> lock(lock_mutex_);

    if(cameras_.empty()) {
        std::cerr << "No devices connected, restart application or verify with 'lsusb'" << std::endl;
        return;
    }

    for (auto &&cam : cameras_)
        cam.second->SetLaser(laser, power);
}

const void MultiCamD400::SetLaser(int index, bool laser, float power) {
    flip_guard<bool> pause(&loop_paused_, true);
    std::lock_guard<std::mutex> lock(lock_mutex_);

    if(cameras_.empty()) {
        std::cerr << "No devices connected, restart application or verify with 'lsusb'" << std::endl;
        return;
    }

    int i = 0;
    for (auto &&cam : cameras_)
        if (index == i++)
            cam.second->SetLaser(laser, power);
}

void MultiCamD400::Available() {
    while (!initialised);
}

const void MultiCamD400::StabiliseExposure() {
    flip_guard<bool> pause(&loop_paused_, true);
    std::lock_guard<std::mutex> lock(lock_mutex_);

    if(cameras_.empty()) {
        std::cerr << "No devices connected, restart application or verify with 'lsusb'" << std::endl;
        return;
    }

    std::vector<std::thread> threads;
    for (auto &&cam : cameras_)
        threads.emplace_back(std::bind([&cam]() { cam.second->StabiliseExposure(); }));
    std::for_each(threads.begin(), threads.end(), [](std::thread &t) { t.join(); });
}

const void MultiCamD400::StabiliseExposure(int index) {
    flip_guard<bool> pause(&loop_paused_, true);
    std::lock_guard<std::mutex> lock(lock_mutex_);

    if(cameras_.empty()) {
        std::cerr << "No devices connected, restart application or verify with 'lsusb'" << std::endl;
        return;
    }

    int i = 0;
    for (auto &&cam : cameras_)
        if (index == i++)
            cam.second->StabiliseExposure();
}

const void MultiCamD400::Pause(bool pause) {
    loop_paused_ = pause;
}