#ifndef STRATCOM_VJOY_INCLUDE_GUARD_HIDREPDESC_HPP_
#define STRATCOM_VJOY_INCLUDE_GUARD_HIDREPDESC_HPP_

#pragma warning(push)
#pragma warning(disable: 4309) // 'initializing' : truncation of constant value
#pragma warning(disable: 4838) // conversion from 'int' to 'const char' requires a narrowing conversion

namespace HidRepDesc {

    char const hid_3Axis_32Buttons[81] ={
        0x05,
        0x01,
        0x15,
        0x00,
        0x09,
        0x04,
        0xA1,
        0x01,
        0x05,
        0x01,
        0x85,
        0x01,
        0x09,
        0x01,
        0x15,
        0x00,
        0x26,
        0xFF,
        0x7F,
        0x75,
        0x20,
        0x95,
        0x01,
        0xA1,
        0x00,
        0x09,
        0x30,
        0x81,
        0x02,
        0x09,
        0x31,
        0x81,
        0x02,
        0x09,
        0x32,
        0x81,
        0x02,
        0x81,
        0x01,
        0x81,
        0x01,
        0x81,
        0x01,
        0x81,
        0x01,
        0x81,
        0x01,
        0xC0,
        0x75,
        0x20,
        0x95,
        0x04,
        0x81,
        0x01,
        0x05,
        0x09,
        0x15,
        0x00,
        0x25,
        0x01,
        0x55,
        0x00,
        0x65,
        0x00,
        0x19,
        0x01,
        0x29,
        0x20,
        0x75,
        0x01,
        0x95,
        0x20,
        0x81,
        0x02,
        0x75,
        0x60,
        0x95,
        0x01,
        0x81,
        0x01,
        0xC0
    };

    char const hid_32Buttons[75] ={
        0x05,
        0x01,
        0x15,
        0x00,
        0x09,
        0x04,
        0xA1,
        0x01,
        0x05,
        0x01,
        0x85,
        0x03,
        0x09,
        0x01,
        0x15,
        0x00,
        0x26,
        0xFF,
        0x7F,
        0x75,
        0x20,
        0x95,
        0x01,
        0xA1,
        0x00,
        0x81,
        0x01,
        0x81,
        0x01,
        0x81,
        0x01,
        0x81,
        0x01,
        0x81,
        0x01,
        0x81,
        0x01,
        0x81,
        0x01,
        0x81,
        0x01,
        0xC0,
        0x75,
        0x20,
        0x95,
        0x04,
        0x81,
        0x01,
        0x05,
        0x09,
        0x15,
        0x00,
        0x25,
        0x01,
        0x55,
        0x00,
        0x65,
        0x00,
        0x19,
        0x01,
        0x29,
        0x20,
        0x75,
        0x01,
        0x95,
        0x20,
        0x81,
        0x02,
        0x75,
        0x60,
        0x95,
        0x01,
        0x81,
        0x01,
        0xC0
    };
}

#pragma warning(pop)

#endif
