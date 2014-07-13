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

namespace {
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
                LOG("Suitable device found at vJoy rId " << rId);
                ret = rId;
                break;
            }
        }
        return ret;
    }
}

struct EventProcessor::ProcessorImpl
{
    stratcom_device* stratcom;
    UINT rId;
    std::atomic<bool> quit_requested;
    EventProcessor* parent;
    Barrier device_error_barrier;

    ProcessorImpl(EventProcessor*);
    ~ProcessorImpl();
    bool initializeStratcom();
    bool initializeVJoy();
    void processInputEvents();
    void requestTerminationProcessLoop();
};

EventProcessor::ProcessorImpl::ProcessorImpl(EventProcessor* n_parent)
    :stratcom(nullptr), rId(0), quit_requested(false), parent(n_parent)
{
    stratcom_init();
}

EventProcessor::ProcessorImpl::~ProcessorImpl()
{
    if(rId) {
        ResetVJD(rId);
        RelinquishVJD(rId);
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

    rId = enumerateVJDevices();
    if(!rId) {
        LOG("No suitable vJoy device found.");
        return false;
    }

    if(!AcquireVJD(rId)) {
        LOG("Could not acquire vJoy device for writing.");
        return false;
    }

    if(!ResetVJD(rId)) {
        return false;
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
                        // does nothing
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
                    emit parent->sliderPositionChanged(slider.status);
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
        pImpl_->device_error_barrier.wait();
        if(pImpl_->quit_requested.load()) {
            break;
        }
        pImpl_->device_error_barrier.reset();
        if(pImpl_->initializeStratcom() && pImpl_->initializeVJoy()) {
            emit deviceInitializedSuccessfully();
            return;
        }
        emit deviceError();
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

EventProcessor::State EventProcessor::getState() const
{
    return State::READY;
}

