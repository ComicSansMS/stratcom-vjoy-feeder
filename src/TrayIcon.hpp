#ifndef STRATCOM_VJOY_INCLUDE_GUARD_TRAY_ICON_HPP_
#define STRATCOM_VJOY_INCLUDE_GUARD_TRAY_ICON_HPP_

#include "IconProvider.hpp"

#include <QSystemTrayIcon>
#include <QMenu>
#include <QWidget>

#include <memory>


class TrayIcon : public QSystemTrayIcon {
    Q_OBJECT
public:
    TrayIcon(QApplication& the_app, QObject* parent = nullptr);
    ~TrayIcon();
public slots:
    void onQuitRequested();
    void onIconClicked();
    void onDeviceInitializedSuccessfully();
    void onDeviceError();
    void onSliderPositionChanged(int new_position);
    void toggleOverlayDisplay(bool doShow);
    void setOptionShiftedButtons(bool doShiftedButtons);
    void setOptionShiftPlusMinus(bool doShiftPlusMinus);
    void onRetryDeviceInit();
signals:
    void quitRequestReceived();
    void deviceInitRequest();
    void optionShiftedButtons(bool doShiftedButtons);
    void optionShiftPlusMinus(bool doShiftPlusMinus);
private:
    void createActions();
    void createMenu();
private:
    QApplication* m_theApp;
    std::unique_ptr<QMenu> m_contextMenu;
    QAction* m_actionQuit;
    QAction* m_actionShiftButtons;
    QAction* m_actionShiftPlusMinus;
    QAction* m_actionRetryDeviceInit;
    QWidget* m_overlayWidget;
    std::unique_ptr<IconProvider> m_iconProvider;
};

#endif
