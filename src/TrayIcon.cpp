#include "TrayIcon.hpp"

#include <QApplication>
#include <QMessageBox>


#include <QWidget>

TrayIcon::TrayIcon(QApplication& the_app, QObject* parent)
    :QSystemTrayIcon(parent), m_theApp(&the_app), m_contextMenu(std::make_unique<QMenu>()), m_actionQuit(nullptr),
     m_actionShiftButtons(nullptr), m_actionShiftPlusMinus(nullptr), m_overlayWidget(nullptr),
     m_iconProvider(std::make_unique<IconProvider>())
{
    createActions();
    createMenu();

    setToolTip("Stratcom VJoy-Feeder");
    setIcon(m_iconProvider->getIcon(IconProvider::ICON_APPLICATION));
    show();

    m_overlayWidget = new QWidget();
    m_overlayWidget->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    m_overlayWidget->setAttribute(Qt::WA_Disabled);
}

TrayIcon::~TrayIcon()
{
    delete m_overlayWidget;
}

void TrayIcon::toggleOverlayDisplay(bool doShow)
{
    if(!doShow) {
        m_overlayWidget->hide();
    }
    else {
        m_overlayWidget->show();
    }
}

void TrayIcon::setOptionShiftedButtons(bool doShiftedButtons)
{
    m_actionShiftPlusMinus->setEnabled(doShiftedButtons);
    emit optionShiftedButtons(doShiftedButtons);
}

void TrayIcon::setOptionShiftPlusMinus(bool doShiftPlusMinus)
{
    emit optionShiftPlusMinus(doShiftPlusMinus);
}

void TrayIcon::createActions()
{
    m_actionQuit = new QAction("Quit", this);
    connect(m_actionQuit, SIGNAL(triggered()), this, SLOT(onQuitRequested()));

    m_actionShiftButtons = new QAction("Use Shift Buttons", this);
    m_actionShiftButtons->setCheckable(true);
    m_actionShiftButtons->setChecked(false);
    connect(m_actionShiftButtons, &QAction::toggled, this, &TrayIcon::setOptionShiftedButtons);

    m_actionShiftPlusMinus = new QAction("Shift Buttons +/-", this);
    m_actionShiftPlusMinus->setCheckable(true);
    m_actionShiftPlusMinus->setChecked(false);
    m_actionShiftPlusMinus->setEnabled(false);
    connect(m_actionShiftPlusMinus, &QAction::toggled, this, &TrayIcon::setOptionShiftPlusMinus);
}

void TrayIcon::createMenu()
{
    m_contextMenu->addAction(m_actionShiftButtons);
    m_contextMenu->addAction(m_actionShiftPlusMinus);
    m_contextMenu->addSeparator();
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
