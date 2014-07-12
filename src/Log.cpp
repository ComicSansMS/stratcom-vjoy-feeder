#include "Log.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <condition_variable>
#include <fstream>
#include <memory>
#include <mutex>
#include <queue>
#include <sstream>
#include <thread>
#include <unordered_map>

namespace {
    std::once_flag g_LoggerSingletonInit;
    std::unique_ptr<Logger> g_LoggerSingleton;
}

class Logger::Pimpl {
private:
    std::unordered_map<std::thread::id, std::unique_ptr<std::stringstream>> m_threadLocalStreams;
    std::mutex m_threadLocalStreamsMutex;

    std::mutex m_messageQueueMutex;
    std::condition_variable m_messageQueueCondition;
    std::queue<std::string> m_logMessages;
    bool m_shutdown;

    std::thread m_logThread;

public:
    Pimpl()
        :m_shutdown(false)
    {
        m_logThread = std::thread([this]() { fileWriteLoop(); });
    }

    ~Pimpl()
    {
    }

    void shutdown()
    {
        {
            std::lock_guard<std::mutex> lk(m_messageQueueMutex);
            m_shutdown = true;
            m_messageQueueCondition.notify_all();
        }
        // std::thread::join() will deadlock in VC<14 if called after main exited
        //https://connect.microsoft.com/VisualStudio/feedback/details/747145
        m_logThread.join();
    }

    std::ostream& getLogStream()
    {
        auto const thread_id = std::this_thread::get_id();
        std::lock_guard<std::mutex> lk(m_threadLocalStreamsMutex);
        auto it = m_threadLocalStreams.find(thread_id);
        if(it == end(m_threadLocalStreams)) {
            it = m_threadLocalStreams.emplace(std::make_pair(thread_id, std::make_unique<std::stringstream>())).first;
        }
        return *(it->second);
    }

    void pushLogMessage(std::string str)
    {
        std::lock_guard<std::mutex> lk(m_messageQueueMutex);
        m_logMessages.emplace(std::move(str));
        m_messageQueueCondition.notify_one();
    }

    void notifyMessageLogged(std::ostream& os)
    {
        std::stringstream tmp;
        tmp.swap(dynamic_cast<std::stringstream&>(os));
        pushLogMessage(tmp.str());
    }
private:
    void fileWriteLoop()
    {
        std::ofstream fout("out.log");
        for(;;) {
            std::string str;
            {
                std::unique_lock<std::mutex> lk(m_messageQueueMutex);
                m_messageQueueCondition.wait(lk, [this]() { return !m_logMessages.empty() || m_shutdown; });
                if(m_shutdown) break;
                str.swap(m_logMessages.front());
                m_logMessages.pop();
            }
            fout << str << std::endl;
            OutputDebugStringA(str.c_str());
            str.clear();
        }
    }
};

Logger::Logger()
    :pimpl_(new Pimpl)
{
}

Logger::~Logger()
{
    pimpl_->shutdown();
}

Logger* Logger::getInstance()
{
    std::call_once(g_LoggerSingletonInit, []() { g_LoggerSingleton.reset(new Logger()); });
    return g_LoggerSingleton.get();
}

void Logger::shutdown()
{
    g_LoggerSingleton.reset();
}

std::ostream& Logger::getLogStream()
{
    return pimpl_->getLogStream();
}

void Logger::notifyMessageLogged(std::ostream& os)
{
    pimpl_->notifyMessageLogged(os);
}

