#ifndef __EventBus_include__
#define __EventBus_include__


#include <lib/util/ArrayList.h>
#include <lib/util/HashMap.h>
#include <lib/util/RingBuffer.h>
#include "kernel/Spinlock.h"
#include "kernel/threads/Thread.h"
#include "kernel/KernelService.h"
#include "kernel/events/Receiver.h"
#include <kernel/events/EventPublisher.h>

class LinearFrameBuffer;

class EventBus : public Thread, public KernelService {

public:

    EventBus();

    EventBus(const EventBus &other) = delete;

    EventBus &operator=(const EventBus &other) = delete;

    ~EventBus() = default;

    void subscribe(Receiver &receiver, uint32_t type);

    void unsubscribe(const Receiver &receiver, uint32_t type);

    void publish(const Event &event);

    void run () override;

    static constexpr char* SERVICE_NAME = "EventBus";

private:

    Util::ArrayList<EventPublisher*> publishers[1024];

    Util::HashMap<const Receiver*, EventPublisher*> receiverMap;

    Util::RingBuffer<const Event*> eventBuffer;

    LinearFrameBuffer *g2d;

    Spinlock lock;

    bool isRunning = true;

    void notify();

    bool isInitialized = false;
};


#endif