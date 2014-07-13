
#include <QApplication>

#include <thread>

#include "TrayIcon.hpp"
#include "EventProcessor.hpp"
#include "Log.hpp"

#include <Windows.h>


int qt_main(int argc, char* argv[], EventProcessor& event_processor)
{
    QApplication theApp(argc, argv);
    TrayIcon ic(theApp);
    ic.connect(&ic, SIGNAL(quitRequestReceived()), &event_processor, SLOT(onQuitRequested()));
    theApp.setQuitOnLastWindowClosed(false);
    return theApp.exec();
}

int main(int argc, char* argv[])
{
    LoggerShutdownToken logger_guard;
    EventProcessor proc;
    std::thread t1([&proc]() { proc.processingLoop(); });
    qt_main(argc, argv, proc);
    t1.join();
    LOG("Shutdown completed.");
    return 0;
}

