#include "TrayIcon.hpp"

#include <QApplication>
#include <QMessageBox>

TrayIcon::TrayIcon(QApplication& the_app, QObject* parent)
    :QSystemTrayIcon(parent), m_theApp(&the_app), m_contextMenu(std::make_unique<QMenu>()), m_actionQuit(nullptr)
{
    createActions();
    m_contextMenu->addAction(m_actionQuit);
    setContextMenu(m_contextMenu.get());
}

void TrayIcon::createActions()
{
    m_actionQuit = new QAction("Quit", this);
    connect(m_actionQuit, SIGNAL(triggered()), this, SLOT(onQuitRequested()));
}

void TrayIcon::onQuitRequested()
{
    m_theApp->quit();
}

void TrayIcon::onIconClicked()
{
    QMessageBox::information(nullptr, "!", "Clicked");
}
