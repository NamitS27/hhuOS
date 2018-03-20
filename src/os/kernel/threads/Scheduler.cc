#include <kernel/Cpu.h>
#include <lib/libc/printf.h>
#include "kernel/threads/Scheduler.h"

extern "C" {
    void Thread_start(Context* first);
    void Thread_switch(Context* current, Context* next);
    void get_thread_vars(uint32_t **current, uint32_t **next);
    void switch_context(Context **current, Context **next);
    void setSchedInit();
    void schedulerYield();
}

Scheduler* Scheduler::scheduler = nullptr;

void schedulerYield() {

    Scheduler::getInstance()->yield();
}

Scheduler::Scheduler () {

    initialized = false;

    get_thread_vars(&currentRegs, &nextRegs);
}

Scheduler *Scheduler::getInstance()  {

    if(scheduler == nullptr) {

        scheduler = new Scheduler();
    }

    return scheduler;
}

void Scheduler::schedule() {

    Thread* first;

    first = readyQueue.pop();

    if (first) {

        currentThread = first;

        initialized = true;

        setSchedInit();

        Thread_start(currentThread->context);
    } else {

        printf("[PANIC] Schedule is empty!\n");
    }
}

void Scheduler::ready(Thread& that) {

    Cpu::disableInterrupts();

    readyQueue.push(&that);

    Cpu::enableInterrupts();
}

void Scheduler::exit() {

    Cpu::disableInterrupts();

    Thread* next = readyQueue.pop();

    if (next == nullptr) {

        printf("[PANIC] Schedule is empty!");

        Cpu::halt();
    }
    
    dispatch (*next);
}

void Scheduler::kill(Thread& that) {

    Cpu::disableInterrupts();

    readyQueue.remove (&that);
    
    Cpu::enableInterrupts();
}

void Scheduler::yield() {

    Cpu::disableInterrupts();

    if (!isThreadWaiting()) {

        Cpu::enableInterrupts();

        return;
    }

    Thread *next = readyQueue.pop();

    readyQueue.push(active());

    dispatch(*next);
}

void Scheduler::block() {

    Thread* next;
    
    Cpu::disableInterrupts();
    
    if (!isThreadWaiting()) {
        printf("Panic: all threads blocked - processor halted.");
        Cpu::halt ();
    }
    
    next = readyQueue.pop();
    
    dispatch (*next);
}

void Scheduler::deblock(Thread &that) {

    Cpu::disableInterrupts();

    readyQueue.push( &that );

    Cpu::enableInterrupts();
}

void Scheduler::dispatch(Thread &next) {

    Thread* current = currentThread;

    currentThread = &next;

    switch_context(&current->context, &next.context);
}

bool Scheduler::isInitialized() {

    return initialized;
}

bool Scheduler::isThreadWaiting() {

    return !readyQueue.isEmpty();
}


