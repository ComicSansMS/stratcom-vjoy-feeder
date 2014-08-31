#ifndef STRATCOM_VJOY_INCLUDE_GUARD_EVENT_PROCESSOR_HPP_
#define STRATCOM_VJOY_INCLUDE_GUARD_EVENT_PROCESSOR_HPP_

#include <QObject>

#include <memory>

class EventProcessor : public QObject
{
    Q_OBJECT
public:
    enum class State {
        READY,
        PROCESSING
    };
private:
    struct ProcessorImpl;
    std::unique_ptr<ProcessorImpl> pImpl_;
public:
    EventProcessor(QObject* parent = nullptr);
    ~EventProcessor();
    void initializeDevices();
    void processingLoop();
    State getState() const;
    void writeConfigToFile(char const* filename);
    void readConfigFromFile(char const* filename);
public slots:
    void onQuitRequested();
    void onDeviceInitRequested();
    void setOptionMapToSingleDevice(bool doMapToSingleDevice);
    void setOptionShiftedButtons(bool doShiftButtons);
    void setOptionShiftPlusMinus(bool doShiftPlusMinus);
signals:
    void deviceInitializedSuccessfully();
    void deviceError();
    void sliderPositionChanged(int new_position);
    void recButtonPressed(bool isPressed);
};


#endif
