
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <public.h>
#include <vjoyinterface.h>
#include <stratcom.h>

#include <iostream>

UINT enumerateVJDevices()
{
    UINT ret = 0;
    for(UINT rId = 1; rId <= 16; ++rId)
    {
        if(GetVJDStatus(rId) == VJD_STAT_FREE) {
            if(GetVJDButtonNumber(rId) < 32) {
                continue;
            }
            if(!GetVJDAxisExist(rId, HID_USAGE_X)) {
                continue;
            }
            if(!GetVJDAxisExist(rId, HID_USAGE_Y)) {
                continue;
            }
            if(!GetVJDAxisExist(rId, HID_USAGE_Z)) {
                continue;
            }
            std::cout << "Suitable device found at vJoy rId " << rId << std::endl;
            ret = rId;
            break;
        }
    }
    return ret;
}

int main(int /* argc */, char* /* argv */[])
{
    stratcom_init();
    if(!vJoyEnabled()) {
        std::cerr << "vJoy not found." << std::endl;
        return 1;
    }

    auto stratcom = stratcom_open_device();
    if(!stratcom) {
        std::cerr << "Strategic Commander device not found." << std::endl;
        return 2;
    }

    UINT const rId = enumerateVJDevices();
    if(!rId) {
        std::cerr << "No suitable vJoy device found." << std::endl;
        return 3;
    }

    if(!AcquireVJD(rId)) {
        std::cerr << "Could not acquire vJoy device for writing." << std::endl;
        return 4;
    }

    auto check = [](stratcom_return ret) {
        if(ret != STRATCOM_RET_SUCCESS) {
            std::cerr << "Error interacting with Strategic Commander device." << std::endl;
            std::exit(5);
        }
    };

    check(stratcom_set_button_led_state(stratcom, STRATCOM_LEDBUTTON_ALL, STRATCOM_LED_OFF));

    check(stratcom_read_input(stratcom));
    stratcom_input_state old_input_state = stratcom_get_input_state(stratcom);

    ResetVJD(rId);

    std::cout << "Up and running. Press REC button on the Strategic Commander to quit." << std::endl;

    UCHAR button_offset = 0;
    bool quitRequested = false;
    while(!quitRequested)
    {
        check(stratcom_read_input(stratcom));
        stratcom_input_state current_input_state = stratcom_get_input_state(stratcom);
        auto input_events = stratcom_create_input_events_from_states(&old_input_state, &current_input_state);

        for(auto it = input_events; it != nullptr; it = it->next) {
            switch(it->type) {
            case STRATCOM_INPUT_EVENT_BUTTON:
                {
                    auto const& button = it->desc.button;
                    switch(button.button) {
                    case STRATCOM_BUTTON_1:
                        SetBtn(button.status, rId, 1 + button_offset);
                        break;
                    case STRATCOM_BUTTON_2:
                        SetBtn(button.status, rId, 2 + button_offset);
                        break;
                    case STRATCOM_BUTTON_3:
                        SetBtn(button.status, rId, 3 + button_offset);
                        break;
                    case STRATCOM_BUTTON_4:
                        SetBtn(button.status, rId, 4 + button_offset);
                        break;
                    case STRATCOM_BUTTON_5:
                        SetBtn(button.status, rId, 5 + button_offset);
                        break;
                    case STRATCOM_BUTTON_6:
                        SetBtn(button.status, rId, 6 + button_offset);
                        break;
                    case STRATCOM_BUTTON_PLUS:
                        SetBtn(button.status, rId, 7 + button_offset);
                        break;
                    case STRATCOM_BUTTON_MINUS:
                        SetBtn(button.status, rId, 8 + button_offset);
                        break;
                    case STRATCOM_BUTTON_SHIFT1:
                        SetBtn(button.status, rId, 9 + button_offset);
                        break;
                    case STRATCOM_BUTTON_SHIFT2:
                        SetBtn(button.status, rId, 10 + button_offset);
                        break;
                    case STRATCOM_BUTTON_SHIFT3:
                        SetBtn(button.status, rId, 11 + button_offset);
                        break;
                    case STRATCOM_BUTTON_REC:
                        quitRequested = true;
                        break;
                    default:
                        break;
                    }
                } break;
            case STRATCOM_INPUT_EVENT_SLIDER:
                {
                    auto const& slider = it->desc.slider;
                    switch(slider.status) {
                    case STRATCOM_SLIDER_1: button_offset = 0;  break;
                    case STRATCOM_SLIDER_2: button_offset = 11; break;
                    case STRATCOM_SLIDER_3: button_offset = 22; break;
                    }
                    ResetButtons(rId);
                } break;
            case STRATCOM_INPUT_EVENT_AXIS:
                {
                    auto const& axis = it->desc.axis;
                    LONG axis_value = (axis.status + 512) * 32;
                    switch(axis.axis) {
                    case STRATCOM_AXIS_X:
                        SetAxis(axis_value, rId, HID_USAGE_X);
                        break;
                    case STRATCOM_AXIS_Y:
                        SetAxis(axis_value, rId, HID_USAGE_Y);
                        break;
                    case STRATCOM_AXIS_Z:
                        SetAxis(axis_value, rId, HID_USAGE_Z);
                        break;
                    }
                } break;
            }
        }
        stratcom_free_input_events(input_events);
        old_input_state = current_input_state;
    }

    std::cout << "Exit requested." << std::endl;

    ResetVJD(rId);

    RelinquishVJD(rId);

    stratcom_close_device(stratcom);

    stratcom_shutdown();

    std::cout << "Shutdown completed." << std::endl;
    return 0;
}

