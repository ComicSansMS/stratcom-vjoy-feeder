#ifndef STRATCOM_VJOY_INCLUDE_GUARD_ICON_PROVIDER_HPP_
#define STRATCOM_VJOY_INCLUDE_GUARD_ICON_PROVIDER_HPP_

#include <QIcon>

#include <vector>


class IconProvider {
public:
    enum IconIdentifier {
        ICON_APPLICATION = 0,
        ICON_TRAY_ERROR,
        ICON_TRAY_SLIDER1,
        ICON_TRAY_SLIDER2,
        ICON_TRAY_SLIDER3,
        ICON_IDENTIFIERS_END
    };
private:
    std::vector<QIcon> m_Icons;
public:
    IconProvider();
    QIcon const& getIcon(IconIdentifier id) const;
};

#endif
