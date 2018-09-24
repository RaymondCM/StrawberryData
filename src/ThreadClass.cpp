#include <iostream>
#include <functional>
#include "ThreadClass.hpp"

ThreadClass::ThreadClass(unsigned int hz) : refresh_rate_(hz), ms_timeout_(1000 / hz) {
    //Start(&ThreadClass::Setup, this); Usage in derived class
}

ThreadClass::~ThreadClass() {
    // Thread terminated when class goes out of scope
    cancel_thread_ = true;
    if(thread_.joinable())
        thread_.join();
}

const bool ThreadClass::StartThread()
{
    thread_ = std::thread(std::bind(&ThreadClass::Setup, this));
}

template<typename _Function_ref, typename _Scope>
const bool ThreadClass::StartThread(_Function_ref &&__f, _Scope __scope)
{
    thread_ = std::thread(std::bind(__f, __scope));
}

const bool ThreadClass::ThreadAlive() {
    return !cancel_thread_ && thread_.joinable();
}

const void ThreadClass::Setup() {
    while (ThreadAlive()) {
        try {
            //Wrap the image capture logic around two time points
            start_ = std::chrono::system_clock::now();
            std::cerr << "No implementation provided for ThreadClass::Setup() calling ::Loop()" << std::endl;
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

const void ThreadClass::Loop() {
    std::cerr << "No implementation provided for ThreadClass::Loop() exiting thread" << std::endl;
    cancel_thread_ = true;
}
