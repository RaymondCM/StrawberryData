#include <iostream>
#include "ThreadClass.h"

ThreadClass::ThreadClass(unsigned int hz) : refresh_rate_(hz), ms_timeout_(1000 / hz) {

}

ThreadClass::~ThreadClass() {
    // Thread terminated when class goes out of scope
    cancel_thread_ = true;
    thread_.join();
}

const bool ThreadClass::ThreadAlive() {
    return !cancel_thread_;
}

const void ThreadClass::Setup() {
    std::cerr << "No implementation provided for ThreadClass::Setup() exiting thread" << std::endl;
    Loop();
}

const void ThreadClass::Loop() {
    std::cerr << "No implementation provided for ThreadClass::Loop() exiting thread" << std::endl;
}
