#ifndef STRATCOM_VJOY_INCLUDE_GUARD_BARRIER_HPP_
#define STRATCOM_VJOY_INCLUDE_GUARD_BARRIER_HPP_

#include <mutex>
#include <condition_variable>

class Barrier {
private:
    std::mutex m_mutex;
    std::condition_variable m_condvar;
    bool m_doContinue;
public:
    Barrier()
        :m_doContinue(false)
    {
    }
    void wait()
    {
        std::unique_lock<std::mutex> lk(m_mutex);
        m_condvar.wait(lk, [this]() { return m_doContinue; });
    }
    void signal()
    {
        std::unique_lock<std::mutex> lk(m_mutex);
        m_doContinue = true;
        m_condvar.notify_all();
    }
    void reset()
    {
        std::unique_lock<std::mutex> lk(m_mutex);
        m_doContinue = false;
    }
};

#endif
