#ifndef STRATCOM_VJOY_INCLUDE_GUARD_CONFIG_HPP_
#define STRATCOM_VJOY_INCLUDE_GUARD_CONFIG_HPP_

#include <chrono>
#include <cstdint>
#include <iosfwd>

struct Config_T {
    bool mapToSingleDevice;
    bool shiftedButtons;
    bool shiftPlusMinus;
    std::uint32_t vjdDeviceRangeFirst;
    std::uint32_t vjdDeviceRangeLast;
    bool automaticRetryOnDeviceFailure;
    std::chrono::milliseconds deviceRetryTimeout;
    Config_T();
};

void serialize(Config_T const& cfg, std::ostream& os);
Config_T deserialize(std::istream& is);

#endif
