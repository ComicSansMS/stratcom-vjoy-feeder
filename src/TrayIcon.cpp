#include "TrayIcon.hpp"

#include <QApplication>
#include <QMessageBox>

#include <QWidget>

TrayIcon::TrayIcon(QApplication& the_app, QObject* parent)
    :QSystemTrayIcon(parent), m_theApp(&the_app), m_contextMenu(std::make_unique<QMenu>()), m_actionQuit(nullptr),
     m_actionMapToSingleDevice(nullptr), m_actionShiftButtons(nullptr), m_actionShiftPlusMinus(nullptr),
     m_overlayWidget(nullptr), m_iconProvider(std::make_unique<IconProvider>())
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

void TrayIcon::setOptionMapToSingleDevice(bool doMapToSingleDevice)
{
    m_actionShiftButtons->setEnabled(!doMapToSingleDevice);
    m_actionShiftPlusMinus->setEnabled(!doMapToSingleDevice && m_actionShiftButtons->isChecked());
    emit optionMapToSingleDevice(doMapToSingleDevice);
}

void TrayIcon::setOptionShiftedButtons(bool doShiftedButtons)
{
    m_actionShiftPlusMinus->setEnabled(!m_actionMapToSingleDevice->isChecked() && doShiftedButtons);
    emit optionShiftedButtons(doShiftedButtons);
}

void TrayIcon::setOptionShiftPlusMinus(bool doShiftPlusMinus)
{
    emit optionShiftPlusMinus(doShiftPlusMinus);
}

void TrayIcon::onRetryDeviceInit()
{
    m_actionRetryDeviceInit->setEnabled(false);
    emit deviceInitRequest();
}

void TrayIcon::onConfigChange(Config_T config)
{
    m_actionMapToSingleDevice->setChecked(config.mapToSingleDevice);
    m_actionShiftButtons->setChecked(config.shiftedButtons);
    m_actionShiftPlusMinus->setChecked(config.shiftPlusMinus);
}

void TrayIcon::createActions()
{
    m_actionQuit = new QAction("Quit", this);
    connect(m_actionQuit, SIGNAL(triggered()), this, SLOT(onQuitRequested()));

    m_actionAbout = new QAction("About...", this);
    connect(m_actionAbout, SIGNAL(triggered()), this, SLOT(onAboutClicked()));

    m_actionMapToSingleDevice = new QAction("Map Sliders to Single Device", this);
    m_actionMapToSingleDevice->setCheckable(true);
    m_actionMapToSingleDevice->setChecked(false);
    connect(m_actionMapToSingleDevice, &QAction::toggled, this, &TrayIcon::setOptionMapToSingleDevice);

    m_actionShiftButtons = new QAction("Use Shift Buttons", this);
    m_actionShiftButtons->setCheckable(true);
    m_actionShiftButtons->setChecked(false);
    connect(m_actionShiftButtons, &QAction::toggled, this, &TrayIcon::setOptionShiftedButtons);

    m_actionShiftPlusMinus = new QAction("Shift Buttons +/-", this);
    m_actionShiftPlusMinus->setCheckable(true);
    m_actionShiftPlusMinus->setChecked(false);
    m_actionShiftPlusMinus->setEnabled(false);
    connect(m_actionShiftPlusMinus, &QAction::toggled, this, &TrayIcon::setOptionShiftPlusMinus);

    m_actionRetryDeviceInit = new QAction("Retry Device Initialization", this);
    m_actionRetryDeviceInit->setEnabled(false);
    m_actionRetryDeviceInit->setVisible(false);
    connect(m_actionRetryDeviceInit, &QAction::triggered, this, &TrayIcon::onRetryDeviceInit);
}

void TrayIcon::createMenu()
{
    m_contextMenu->addAction(m_actionRetryDeviceInit);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_actionMapToSingleDevice);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_actionShiftButtons);
    m_contextMenu->addAction(m_actionShiftPlusMinus);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_actionAbout);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_actionQuit);
    setContextMenu(m_contextMenu.get());
}

void TrayIcon::onQuitRequested()
{
    emit quitRequestReceived();
    m_theApp->quit();
}

void TrayIcon::onAboutClicked()
{
    QMessageBox msgbox;
    msgbox.setWindowTitle("Stratcom VJoy Feeder");
    msgbox.setText("Stratcom VJoy Feeder\n(C) 2014-2018 Andreas Weis\nhttp://www.ghulbus-inc.de/\n\nLicensed under GPL v3.\n\n"
                   "This software was built using Qt5 (http://www.qt.io/).\n");
    msgbox.setStandardButtons(QMessageBox::Ok);
    msgbox.addButton("About Qt...", QMessageBox::YesRole);
    msgbox.setDefaultButton(QMessageBox::Ok);
    auto const res = msgbox.exec();
    if (res != QMessageBox::Ok) {
        QMessageBox::aboutQt(nullptr);
    }
}

void TrayIcon::onDeviceInitializedSuccessfully()
{
    showMessage("Stratcom VJoy-Feeder", "Stratcom VJoy-Feeder is running.");
    m_actionRetryDeviceInit->setEnabled(false);
    m_actionRetryDeviceInit->setVisible(false);
    m_actionMapToSingleDevice->setVisible(true);
    m_actionShiftButtons->setVisible(true);
    m_actionShiftPlusMinus->setVisible(true);
    setIcon(m_iconProvider->getIcon(IconProvider::ICON_APPLICATION));
}

void TrayIcon::onDeviceError()
{
    showMessage("Stratcom VJoy-Feeder", "Device error.", Warning);
    m_actionMapToSingleDevice->setVisible(false);
    m_actionShiftButtons->setVisible(false);
    m_actionShiftPlusMinus->setVisible(false);
    m_actionRetryDeviceInit->setEnabled(true);
    m_actionRetryDeviceInit->setVisible(true);
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
