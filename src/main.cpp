
#include <QApplication>

#include <thread>

#include "EventProcessor.hpp"
#include "Log.hpp"
#include "TrayIcon.hpp"

#include <Windows.h>

int qt_main(int argc, char* argv[], EventProcessor& event_processor)
{
    QApplication theApp(argc, argv);
    TrayIcon ic(theApp);
    QObject::connect(&ic, &TrayIcon::quitRequestReceived,
                     &event_processor, &EventProcessor::onQuitRequested);
    QObject::connect(&ic, &TrayIcon::deviceInitRequest,
                     &event_processor, &EventProcessor::onDeviceInitRequested);
    QObject::connect(&event_processor, &EventProcessor::deviceInitializedSuccessfully,
                     &ic, &TrayIcon::onDeviceInitializedSuccessfully);
    QObject::connect(&event_processor, &EventProcessor::deviceError,
                     &ic, &TrayIcon::onDeviceError);
    QObject::connect(&event_processor, &EventProcessor::sliderPositionChanged,
                     &ic, &TrayIcon::onSliderPositionChanged);
    QObject::connect(&event_processor, &EventProcessor::recButtonPressed,
                     &ic, &TrayIcon::toggleOverlayDisplay);
    QObject::connect(&ic, &TrayIcon::optionMapToSingleDevice,
                     &event_processor, &EventProcessor::setOptionMapToSingleDevice);
    QObject::connect(&ic, &TrayIcon::optionShiftedButtons,
                     &event_processor, &EventProcessor::setOptionShiftedButtons);
    QObject::connect(&ic, &TrayIcon::optionShiftPlusMinus,
                     &event_processor, &EventProcessor::setOptionShiftPlusMinus);
    emit ic.deviceInitRequest();
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

