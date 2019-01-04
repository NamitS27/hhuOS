#include <kernel/Kernel.h>
#include "InterruptHandler.h"
#include "InterruptManager.h"

InterruptManager &InterruptManager::getInstance() {
    static InterruptManager instance;

    return instance;
}

void InterruptManager::registerInterruptHandler(InterruptHandler *device) {
    lock.acquire();

    interruptHandler.add(device);

    lock.release();
}

void InterruptManager::deregisterInterruptHandler(InterruptHandler *device) {
    lock.acquire();

    interruptHandler.remove(device);

    lock.release();
}

void InterruptManager::run() {
    while(true) {
        lock.acquire();

        for (uint32_t i = 0; i < interruptHandler.size(); i++) {
            if (interruptHandler.get(i)->hasInterruptData()) {
                interruptHandler.get(i)->parseInterruptData();
            }
        }

        lock.release();
    }
}
