#ifndef STRATCOM_VJOY_INCLUDE_GUARD_BARRIER_HPP_
#define STRATCOM_VJOY_INCLUDE_GUARD_BARRIER_HPP_

#include <mutex>
#include <condition_variable>

/*! A very simple, asymmetric barrier for snychronizing threads.
 */
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
    /*! Block until the barrier is signaled.
     */
    void wait()
    {
        std::unique_lock<std::mutex> lk(m_mutex);
        m_condvar.wait(lk, [this]() { return m_doContinue; });
    }
    /*! Block until the barrier is signaled or the timeout expires.
     */
    template<class Rep, class Period>
    bool wait_for(std::chrono::duration<Rep, Period> const& wait_time)
    {
        std::unique_lock<std::mutex> lk(m_mutex);
        return m_condvar.wait_for(lk, wait_time, [this]() { return m_doContinue; });
    }
    /*! Signal all currently waiting threads.
     */
    void signal()
    {
        std::unique_lock<std::mutex> lk(m_mutex);
        m_doContinue = true;
        m_condvar.notify_all();
    }
    /*! Reset the barrier. This is only safe to call if no threads are currently waiting.
     */
    void reset()
    {
        std::unique_lock<std::mutex> lk(m_mutex);
        m_doContinue = false;
    }
};

#endif
