#include "EventProcessor.hpp"

#include "Barrier.hpp"
#include "Log.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <public.h>
#include <vjoyinterface.h>

#include <stratcom.h>

#include <atomic>
#include <chrono>
#include <tuple>

namespace {
    std::tuple<UINT, UINT, UINT> enumerateVJDevices(UINT range_first, UINT range_last)
    {
        UINT first = 0;
        UINT second = 0;
        UINT third = 0;
        for(UINT rId = range_first; rId <= range_last; ++rId)
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
                LOG("Suitable device found at vJoy rId " << rId);
                first = rId;
                break;
            }
        }
        for(UINT rId = range_first; rId <= range_last; ++rId)
        {
            if(GetVJDStatus(rId) == VJD_STAT_FREE) {
                if(GetVJDButtonNumber(rId) < 32) {
                    continue;
                }
                if(rId == first) {
                    continue;
                }
                LOG("Suitable secondary device found at vJoy rId " << rId);
                second = rId;
                break;
            }
        }
        for(UINT rId = range_first; rId <= range_last; ++rId)
        {
            if(GetVJDStatus(rId) == VJD_STAT_FREE) {
                if(GetVJDButtonNumber(rId) < 32) {
                    continue;
                }
                if((rId == first) || (rId == second)) {
                    continue;
                }
                LOG("Suitable ternary device found at vJoy rId " << rId);
                third = rId;
                break;
            }
        }
        return std::make_tuple(first, second, third);
    }
}

struct Config_T {
    std::atomic<bool> mapToSingleDevice;
    std::atomic<bool> shiftedButtons;
    std::atomic<bool> shiftPlusMinus;
    UINT vjdDeviceRangeFirst;
    UINT vjdDeviceRangeLast;
    std::atomic<bool> automaticRetryOnDeviceFailure;
    std::chrono::milliseconds deviceRetryTimeout;
    Config_T()
        :mapToSingleDevice(false), shiftedButtons(false), shiftPlusMinus(false),
         vjdDeviceRangeFirst(1), vjdDeviceRangeLast(16),
         automaticRetryOnDeviceFailure(false), deviceRetryTimeout(2000)
    {}
};

struct EventProcessor::ProcessorImpl
{
    stratcom_device* stratcom;
    UINT rId1;
    UINT rId2;
    UINT rId3;
    std::atomic<bool> quit_requested;
    EventProcessor* parent;
    Barrier device_error_barrier;
    Config_T config;

    ProcessorImpl(EventProcessor*);
    ~ProcessorImpl();
    bool initializeStratcom();
    bool initializeVJoy();
    void processInputEvents();
    void requestTerminationProcessLoop();
};

EventProcessor::ProcessorImpl::ProcessorImpl(EventProcessor* n_parent)
    :stratcom(nullptr), rId1(0), rId2(0), rId3(0), quit_requested(false), parent(n_parent)
{
    stratcom_init();
}

EventProcessor::ProcessorImpl::~ProcessorImpl()
{
    if(rId1) {
        ResetVJD(rId1);
        RelinquishVJD(rId1);
    }
    if(rId2) {
        ResetVJD(rId2);
        RelinquishVJD(rId2);
    }
    if(rId3) {
        ResetVJD(rId3);
        RelinquishVJD(rId3);
    }

    if(stratcom) {
        stratcom_set_button_led_state(stratcom, STRATCOM_LEDBUTTON_ALL, STRATCOM_LED_OFF);
        stratcom_close_device(stratcom);
        stratcom_shutdown();
    }
}

bool EventProcessor::ProcessorImpl::initializeStratcom()
{
    stratcom = stratcom_open_device();
    if(!stratcom) {
        LOG("Strategic Commander device not found.");
        return false;
    }

    if(stratcom_set_button_led_state(stratcom, STRATCOM_LEDBUTTON_ALL, STRATCOM_LED_OFF) != STRATCOM_RET_SUCCESS) {
        stratcom_close_device(stratcom);
        stratcom = nullptr;
        return false;
    }
    return true;
}

bool EventProcessor::ProcessorImpl::initializeVJoy()
{
    if(!vJoyEnabled()) {
        LOG("vJoy not found.");
        return false;
    }

    std::tie(rId1, rId2, rId3) = enumerateVJDevices(config.vjdDeviceRangeFirst, config.vjdDeviceRangeLast);
    if(!rId1) {
        LOG("No suitable vJoy device found.");
        rId1 = rId2 = rId3 = 0;
        return false;
    }

    if(!AcquireVJD(rId1)) {
        LOG("Could not acquire primary vJoy device for writing.");
        rId1 = rId2 = rId3 = 0;
        return false;
    }
    if(!ResetVJD(rId1)) {
        LOG("Could not write to primary vJoy device.");
        RelinquishVJD(rId1);
        rId1 = rId2 = rId3 = 0;
        return false;
    }

    if(rId2) {
        if(!AcquireVJD(rId2) || !ResetVJD(rId2)) {
            LOG("Could not write to secondary vJoy device.");
            RelinquishVJD(rId2);
            rId2 = 0;
        }
    }
    if(rId3) {
        if(!AcquireVJD(rId3) || !ResetVJD(rId3)) {
            LOG("Could not write to ternary vJoy device.");
            RelinquishVJD(rId3);
            rId3 = 0;
        }
    }
    return true;
}

void EventProcessor::ProcessorImpl::requestTerminationProcessLoop()
{
    quit_requested.store(true);
}

void EventProcessor::ProcessorImpl::processInputEvents()
{
    auto check = [](stratcom_return ret) {
        if(ret == STRATCOM_RET_ERROR) {
            LOG("Error interacting with Strategic Commander device.");
            return;
        }
    };

    LOG("Stratcom VJoy-Feeder up and running.");

    UCHAR button_offset = 0;
    UINT target_device = rId1;
    stratcom_input_state old_input_state;
    bool first_iteration = true;
    while(!quit_requested.load())
    {
        auto read_result = stratcom_read_input_with_timeout(stratcom, 500);
        check(read_result);

        if(read_result != STRATCOM_RET_NO_DATA) {
            stratcom_input_state current_input_state = stratcom_get_input_state(stratcom);
            if(first_iteration) {
                old_input_state = current_input_state;
                first_iteration = false;
                emit parent->sliderPositionChanged(current_input_state.slider);
                continue;
            }
            auto input_events = stratcom_create_input_events_from_states(&old_input_state, &current_input_state);

            for(auto it = input_events; it != nullptr; it = it->next) {
                switch(it->type) {
                case STRATCOM_INPUT_EVENT_BUTTON:
                {
                    auto const& button = it->desc.button;
                    switch(button.button) {
                    case STRATCOM_BUTTON_1:
                        SetBtn(button.status, target_device, 1 + button_offset);
                        break;
                    case STRATCOM_BUTTON_2:
                        SetBtn(button.status, target_device, 2 + button_offset);
                        break;
                    case STRATCOM_BUTTON_3:
                        SetBtn(button.status, target_device, 3 + button_offset);
                        break;
                    case STRATCOM_BUTTON_4:
                        SetBtn(button.status, target_device, 4 + button_offset);
                        break;
                    case STRATCOM_BUTTON_5:
                        SetBtn(button.status, target_device, 5 + button_offset);
                        break;
                    case STRATCOM_BUTTON_6:
                        SetBtn(button.status, target_device, 6 + button_offset);
                        break;
                    case STRATCOM_BUTTON_PLUS:
                        SetBtn(button.status, target_device, 7 +
                                ((config.mapToSingleDevice || config.shiftPlusMinus) ? button_offset : 0));
                        break;
                    case STRATCOM_BUTTON_MINUS:
                        SetBtn(button.status, target_device, 8 +
                                ((config.mapToSingleDevice || config.shiftPlusMinus) ? button_offset : 0));
                        break;
                    case STRATCOM_BUTTON_SHIFT1:
                        if(!config.mapToSingleDevice && config.shiftedButtons) {
                            button_offset = (button.status) ? 8 : 0;
                            ResetButtons(target_device);
                        }
                        else {
                            SetBtn(button.status, target_device, 25);
                        }
                        break;
                    case STRATCOM_BUTTON_SHIFT2:
                        if(!config.mapToSingleDevice && config.shiftedButtons) {
                            button_offset = (button.status) ? 16 : 0;
                            ResetButtons(target_device);
                        }
                        else {
                            SetBtn(button.status, target_device, 26);
                        }
                        break;
                    case STRATCOM_BUTTON_SHIFT3:
                        if(!config.mapToSingleDevice && config.shiftedButtons) {
                            button_offset = (button.status) ? 24 : 0;
                            ResetButtons(target_device);
                        }
                        else {
                            SetBtn(button.status, target_device, 27);
                        }
                        break;
                    case STRATCOM_BUTTON_REC:
                        emit parent->recButtonPressed(button.status);
                        break;
                    default:
                        break;
                    }
                } break;
                case STRATCOM_INPUT_EVENT_SLIDER:
                {
                    auto const& slider = it->desc.slider;
                    if(config.mapToSingleDevice) {
                        switch(slider.status) {
                        case STRATCOM_SLIDER_1: button_offset = 0; break;
                        case STRATCOM_SLIDER_2: button_offset = 8; break;
                        case STRATCOM_SLIDER_3: button_offset = 16; break;
                        }
                    } else {
                        switch(slider.status) {
                        case STRATCOM_SLIDER_1: target_device = rId1;  break;
                        case STRATCOM_SLIDER_2: if(rId2) { target_device = rId2; } break;
                        case STRATCOM_SLIDER_3: if(rId3) { target_device = rId3; } break;
                        }
                    }
                    emit parent->sliderPositionChanged(slider.status);
                    ResetButtons(rId1);
                    if(rId2) { ResetButtons(rId2); }
                    if(rId3) { ResetButtons(rId3); }
                } break;
                case STRATCOM_INPUT_EVENT_AXIS:
                {
                    auto const& axis = it->desc.axis;
                    LONG axis_value = (axis.status + 512) * 32;
                    switch(axis.axis) {
                    case STRATCOM_AXIS_X:
                        SetAxis(axis_value, rId1, HID_USAGE_X);
                        break;
                    case STRATCOM_AXIS_Y:
                        SetAxis(axis_value, rId1, HID_USAGE_Y);
                        break;
                    case STRATCOM_AXIS_Z:
                        SetAxis(axis_value, rId1, HID_USAGE_Z);
                        break;
                    }
                } break;
                }
            }
            stratcom_free_input_events(input_events);
            old_input_state = current_input_state;
        }
    }
    LOG("Terminating event processing loop.");
}

EventProcessor::EventProcessor(QObject* parent)
    :QObject(parent), pImpl_(std::make_unique<EventProcessor::ProcessorImpl>(this))
{
}

EventProcessor::~EventProcessor()
{
}

void EventProcessor::initializeDevices()
{
    for(;;) {
        bool const retryWasRequested =
            (pImpl_->config.automaticRetryOnDeviceFailure ?
             pImpl_->device_error_barrier.wait_for(pImpl_->config.deviceRetryTimeout) :
             (pImpl_->device_error_barrier.wait(), true));
        if(pImpl_->quit_requested.load()) {
            break;
        }
        pImpl_->device_error_barrier.reset();
        if(pImpl_->initializeStratcom() && pImpl_->initializeVJoy()) {
            emit deviceInitializedSuccessfully();
            return;
        }
        if(retryWasRequested) {
            emit deviceError();
        }
    }
}

void EventProcessor::processingLoop()
{
    while(!pImpl_->quit_requested.load()) {
        initializeDevices();
        pImpl_->processInputEvents();
    }
}

void EventProcessor::onQuitRequested()
{
    LOG("Exit requested.");
    pImpl_->requestTerminationProcessLoop();
    pImpl_->device_error_barrier.signal();
}

void EventProcessor::onDeviceInitRequested()
{
    pImpl_->device_error_barrier.signal();
}

void EventProcessor::setOptionMapToSingleDevice(bool doMapToSingleDevice)
{
    pImpl_->config.mapToSingleDevice = doMapToSingleDevice;
}

void EventProcessor::setOptionShiftedButtons(bool doShiftButtons)
{
    pImpl_->config.shiftedButtons = doShiftButtons;
}

void EventProcessor::setOptionShiftPlusMinus(bool doShiftPlusMinus)
{
    pImpl_->config.shiftPlusMinus = doShiftPlusMinus;
}

EventProcessor::State EventProcessor::getState() const
{
    return State::READY;
}

