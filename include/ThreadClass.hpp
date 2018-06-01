#ifndef STRAWBERRYDATA_THREADCLASS_H
#define STRAWBERRYDATA_THREADCLASS_H


#include <thread>
#include <mutex>

class ThreadClass {
public:
    explicit ThreadClass(unsigned int hz = 60);
    ~ThreadClass();
    virtual const bool ThreadAlive();
private:
    //Thread parameters
    std::thread thread_;
    bool cancel_thread_ = false;
    std::mutex lock_mutex_;

    //Thread bound functions
    virtual const void Setup();
    virtual const void Loop();

    //Thread timers
    size_t refresh_rate_ = 100;
    std::chrono::system_clock::time_point start_, end_;
    long long int thread_timeout_, ms_timeout_;
};


#endif //STRAWBERRYDATA_THREADCLASS_H
