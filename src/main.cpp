
#include <QApplication>

#include <mutex>
#include <condition_variable>
#include <thread>

#include "TrayIcon.hpp"
#include "EventProcessor.hpp"
#include "Log.hpp"

#include <Windows.h>

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
};


int qt_main(int argc, char* argv[], EventProcessor& event_processor, Barrier& barr)
{
    QApplication theApp(argc, argv);
    TrayIcon ic(theApp);
    QObject::connect(&ic, &TrayIcon::quitRequestReceived,
                     &event_processor, &EventProcessor::onQuitRequested);
    QObject::connect(&event_processor, &EventProcessor::deviceInitializedSuccessfully,
                     &ic, &TrayIcon::onDeviceInitializedSuccessfully);
    QObject::connect(&event_processor, &EventProcessor::deviceError,
                     &ic, &TrayIcon::onDeviceError);
    QObject::connect(&event_processor, &EventProcessor::sliderPositionChanged,
                     &ic, &TrayIcon::onSliderPositionChanged);
    barr.signal();
    theApp.setQuitOnLastWindowClosed(false);
    return theApp.exec();
}

int main(int argc, char* argv[])
{
    LoggerShutdownToken logger_guard;
    EventProcessor proc;
    Barrier barr;
    std::thread t1([&proc, &barr]() { barr.wait(); proc.processingLoop(); });
    qt_main(argc, argv, proc, barr);
    t1.join();
    LOG("Shutdown completed.");
    return 0;
}

