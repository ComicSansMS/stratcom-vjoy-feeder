#include "TrayIcon.hpp"

#include <QApplication>
#include <QMessageBox>

TrayIcon::TrayIcon(QApplication& the_app, QObject* parent)
    :QSystemTrayIcon(parent), m_theApp(&the_app), m_contextMenu(std::make_unique<QMenu>()), m_actionQuit(nullptr),
     m_iconProvider(std::make_unique<IconProvider>())
{
    createActions();
    createMenu();

    setToolTip("Stratcom VJoy-Feeder");
    setIcon(m_iconProvider->getIcon(IconProvider::ICON_APPLICATION));
    show();
}

void TrayIcon::createActions()
{
    m_actionQuit = new QAction("Quit", this);
    connect(m_actionQuit, SIGNAL(triggered()), this, SLOT(onQuitRequested()));
}

void TrayIcon::createMenu()
{
    m_contextMenu->addAction(m_actionQuit);
    setContextMenu(m_contextMenu.get());
}

void TrayIcon::onQuitRequested()
{
    emit quitRequestReceived();
    m_theApp->quit();
}

void TrayIcon::onDeviceInitializedSuccessfully()
{
    showMessage("Stratcom VJoy-Feeder", "Stratcom VJoy-Feeder is running.");
}

void TrayIcon::onDeviceError()
{
    showMessage("Stratcom VJoy-Feeder", "Device error.", Warning);
    setIcon(m_iconProvider->getIcon(IconProvider::ICON_TRAY_ERROR));
}

void TrayIcon::onSliderPositionChanged(int new_position)
{
    auto const new_icon = (new_position == 1) ? IconProvider::ICON_TRAY_SLIDER1 : 
                          ((new_position == 2) ? IconProvider::ICON_TRAY_SLIDER2 :
                          IconProvider::ICON_TRAY_SLIDER3);
    setIcon(m_iconProvider->getIcon(new_icon));
}

void TrayIcon::onIconClicked()
{
    QMessageBox::information(nullptr, "!", "Clicked");
}
