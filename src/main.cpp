
#include <QApplication>
#include <QtWinExtras>

#include <thread>

#include "TrayIcon.hpp"
#include "EventProcessor.hpp"
#include "Log.hpp"

#include <Windows.h>
#include <resource.h>


int qt_main(int argc, char* argv[])
{
    QApplication theApp(argc, argv);

    TrayIcon ic(theApp);
    ic.setToolTip("Stratcom VJoy-Feeder");
    QIcon icon("../gfx/stratcom.ico");

    HICON hicon = LoadIcon(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_ICON_ERROR));
    auto err = GetLastError();

    HICON hicon2 = (HICON)LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_ICON_ERROR), IMAGE_ICON, 16, 16, 0);
    QIcon icon2(QtWin::fromHICON(hicon2));
    DestroyIcon(hicon2);


    ic.setIcon(icon2);
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

