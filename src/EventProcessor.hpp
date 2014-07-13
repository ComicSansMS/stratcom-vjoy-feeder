#ifndef STRATCOM_VJOY_INCLUDE_GUARD_EVENT_PROCESSOR_HPP_
#define STRATCOM_VJOY_INCLUDE_GUARD_EVENT_PROCESSOR_HPP_

#include <QObject>

class EventProcessor : public QObject
{
    Q_OBJECT
public:
    EventProcessor(QObject* parent = nullptr);
    int processingLoop();
private slots:
};


#endif
