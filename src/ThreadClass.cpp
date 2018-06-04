#include <iostream>
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
    return !cancel_thread_;
}

const void ThreadClass::Setup() {
    std::cerr << "No implementation provided for ThreadClass::Setup() calling ::Loop()" << std::endl;
    Loop();
}

const void ThreadClass::Loop() {
    std::cerr << "No implementation provided for ThreadClass::Loop() exiting thread" << std::endl;
}
