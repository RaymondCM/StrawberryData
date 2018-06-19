#ifndef STRAWBERRYDATA_THREADCLASS_H
#define STRAWBERRYDATA_THREADCLASS_H

#include <thread>
#include <mutex>

/// Usage:
///     Should be derived and both functions below should be implemented in the child class
///             virtual const void Setup();
///             virtual const void Loop();
///     Thread can be started by calling ::Start, alternately you can define custom entry point with ::Start(_t, _tp)
/// class Example : public ThreadClass {
///    const void Setup() override {Loop();};
///    const void Loop() override {};
/// };

class ThreadClass {
public:
    explicit ThreadClass(unsigned int hz = 60);

    virtual ~ThreadClass();
    const bool ThreadAlive();

protected:
    //Thread parameters
    std::thread thread_;
    bool cancel_thread_ = false;
    std::mutex lock_mutex_;

    //Thread bound functions
    virtual const void Setup();
    virtual const void Loop();

    //Thread timers for extended implementation
    size_t refresh_rate_ = 100;
    std::chrono::system_clock::time_point start_, end_;
    long long int sleep_for_, ms_timeout_;

    //Initialisation Parameters
    const bool StartThread();
    template<typename _Function_ref, typename _Scope>
    const bool StartThread(_Function_ref &&__f, _Scope __scope);
};


#endif //STRAWBERRYDATA_THREADCLASS_H
