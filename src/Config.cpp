
#include "Config.hpp"

#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <istream>
#include <ostream>

Config_T::Config_T()
    :mapToSingleDevice(false), shiftedButtons(false), shiftPlusMinus(false),
    vjdDeviceRangeFirst(1), vjdDeviceRangeLast(16),
    automaticRetryOnDeviceFailure(false), deviceRetryTimeout(2000)
{}

void serialize(Config_T const& cfg, std::ostream& os)
{
    boost::property_tree::ptree pt;
    pt.put("Config.MapAllSlidersToSingleDevice", cfg.mapToSingleDevice);
    pt.put("Config.ShiftedButtons", cfg.shiftedButtons);
    pt.put("Config.ShiftPlusMinusButtons", cfg.shiftPlusMinus);
    pt.put("Config.SmallestVJoyDeviceId", cfg.vjdDeviceRangeFirst);
    pt.put("Config.LargestVJoyDeviceId", cfg.vjdDeviceRangeLast);
    pt.put("Config.AutomaticRetryOnDeviceFailure", cfg.automaticRetryOnDeviceFailure);
    pt.put("Config.TimeIntervalForAutomaticRetryMilliseconds", cfg.deviceRetryTimeout.count());
    boost::property_tree::write_ini(os, pt);
}

template<typename T>
void read_cfg_value(boost::property_tree::ptree const& pt, char const* str, T& v)
{
    v = pt.get<T>(str, v);
}

Config_T deserialize(std::istream& is)
{
    Config_T cfg;
    boost::property_tree::ptree pt;
    boost::property_tree::read_ini(is, pt);
    read_cfg_value(pt, "Config.MapAllSlidersToSingleDevice", cfg.mapToSingleDevice);
    read_cfg_value(pt, "Config.ShiftedButtons", cfg.shiftedButtons);
    read_cfg_value(pt, "Config.ShiftPlusMinusButtons", cfg.shiftPlusMinus);
    read_cfg_value(pt, "Config.SmallestVJoyDeviceId", cfg.vjdDeviceRangeFirst);
    read_cfg_value(pt, "Config.LargestVJoyDeviceId", cfg.vjdDeviceRangeLast);
    read_cfg_value(pt, "Config.AutomaticRetryOnDeviceFailure", cfg.automaticRetryOnDeviceFailure);
    auto tmp = cfg.deviceRetryTimeout.count();
    read_cfg_value(pt, "Config.TimeIntervalForAutomaticRetryMilliseconds", tmp);
    cfg.deviceRetryTimeout = std::chrono::milliseconds(tmp);
    return cfg;
}
