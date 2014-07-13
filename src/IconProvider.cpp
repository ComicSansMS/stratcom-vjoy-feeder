#include "IconProvider.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <resource.h>

#include <QIcon>
#include <QtWinExtras>

#include <cassert>
#include <stdexcept>

namespace {
    QIcon LoadQtIconFromResource(int resource_id)
    {
        HICON hicon = LoadIcon(GetModuleHandle(nullptr), MAKEINTRESOURCE(resource_id));
        if(!hicon) {
            throw std::runtime_error("Unable to load icon.");
        }
        return QIcon(QtWin::fromHICON(hicon));
    }

    QIcon LoadQtIconFromResource(int resource_id, int size)
    {
        HICON hicon = static_cast<HICON>(LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(resource_id),
                                                   IMAGE_ICON, size, size, LR_DEFAULTCOLOR));
        if(!hicon) {
            throw std::runtime_error("Unable to load icon.");
        }
        QIcon ret(QtWin::fromHICON(hicon));
        DestroyIcon(hicon);
        return ret;
    }

    QIcon LoadQtIcon(int i)
    {
        switch(i)
        {
        default: assert(false); return QIcon();
        case IconProvider::ICON_APPLICATION:  return LoadQtIconFromResource(IDI_ICON1);
        case IconProvider::ICON_TRAY_ERROR:   return LoadQtIconFromResource(IDI_ICON_ERROR, 16);
        case IconProvider::ICON_TRAY_SLIDER1: return LoadQtIconFromResource(IDI_ICON_SLIDER1, 16);
        case IconProvider::ICON_TRAY_SLIDER2: return LoadQtIconFromResource(IDI_ICON_SLIDER2, 16);
        case IconProvider::ICON_TRAY_SLIDER3: return LoadQtIconFromResource(IDI_ICON_SLIDER3, 16);
        }
    }
}


IconProvider::IconProvider()
{
    for(int i=0; i < ICON_IDENTIFIERS_END; ++i)
    {
        m_Icons.push_back(LoadQtIcon(i));
    }
}

QIcon const& IconProvider::getIcon(IconIdentifier id) const
{
    return m_Icons.at(id);
}

