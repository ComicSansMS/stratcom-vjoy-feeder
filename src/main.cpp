
#include <QApplication>

#include <thread>

#include "TrayIcon.hpp"
#include "EventProcessor.hpp"
#include "Log.hpp"

int qt_main(int argc, char* argv[])
{
    QApplication theApp(argc, argv);

    TrayIcon ic(theApp);
    ic.setToolTip("Stratcom VJoy-Feeder");
    QIcon icon("../gfx/stratcom.ico");
    ic.setIcon(icon);
    ic.show();
    ic.showMessage("Stratcom VJoy-Feeder", "Stratcom VJoy-Feeder is running.");
    theApp.setQuitOnLastWindowClosed(false);
    return theApp.exec();
}

int main(int argc, char* argv[])
{
    LoggerShutdownToken logger_guard;
    EventProcessor proc;
    std::thread t1([&proc]() { proc.processingLoop(); });
    qt_main(argc, argv);
    t1.join();
    LOG("Shutdown completed.");
    return 0;
}

