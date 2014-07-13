#ifndef STRATCOM_VJOY_INCLUDE_GUARD_TRAY_ICON_HPP_
#define STRATCOM_VJOY_INCLUDE_GUARD_TRAY_ICON_HPP_

#include "IconProvider.hpp"

#include <QSystemTrayIcon>
#include <QMenu>

#include <memory>


class TrayIcon : public QSystemTrayIcon {
    Q_OBJECT
public:
    TrayIcon(QApplication& the_app, QObject* parent = nullptr);
private slots:
    void onQuitRequested();
    void onIconClicked();
signals:
    void quitRequestReceived();
private:
    void createActions();
    void createMenu();
private:
    QApplication* m_theApp;
    std::unique_ptr<QMenu> m_contextMenu;
    QAction* m_actionQuit;
    std::unique_ptr<IconProvider> m_iconProvider;
};

#endif
