/**
 * Main function - is called from assembler code. 
 * 
 * @author Michael Schoettner, Burak Akguel, Christian Gesse, Fabian Ruhland, Filip Krakowski
 * @date HHU, 2018
 */


#include <kernel/threads/IdleThread.h>

#include "user/Application.h"
#include "kernel/services/StorageService.h"
#include "devices/block/storage/AhciDevice.h"
#include <kernel/services/ModuleLoader.h>
#include <devices/graphics/lfb/VesaGraphics.h>
#include <devices/graphics/lfb/CgaGraphics.h>
#include <kernel/services/InputService.h>
#include <kernel/services/StdStreamService.h>
#include <kernel/services/SoundService.h>
#include <devices/graphics/text/VesaText.h>
#include <kernel/Cpu.h>
#include <kernel/Bios.h>
#include <devices/Pit.h>
#include <kernel/interrupts/Pic.h>
#include <kernel/threads/Scheduler.h>
#include <lib/util/HashMap.h>
#include <lib/util/BlockingQueue.h>
#include <lib/Bmp.h>
#include "lib/libc/printf.h"
#include <lib/libc/snprintf.h>
#include <kernel/services/GraphicsService.h>
#include "kernel/memory/SystemManagement.h"
#include "bootlogo.h"

#define DEBUG_MODE 0

#define VERSION "0.1"

extern char *gitversion;

extern "C" {
    void mm_init();
}

IdleThread *idleThread;
EventBus *eventBus;
Application *app;
LinearFrameBuffer *lfb = nullptr;
TextDriver *text = nullptr;

void updateBootScreen(uint8_t percentage, const char *currentActivity) {
    String versionString("hhuOS ");
    versionString += VERSION;
    versionString += " - git ";
    versionString += gitversion;

    auto normalizedPercentage = static_cast<uint8_t>((percentage * 60) / 100);

    lfb->fillRect(0, 0, lfb->getResX(), lfb->getResY(), Colors::HHU_DARK_BLUE);

    lfb->placeString(sun_font_8x16, 50, 10, static_cast<char *>(versionString), Colors::HHU_GRAY, Colors::INVISIBLE);

    if(lfb->getResY() < 350) {
        lfb->placeSprite(50, 45, static_cast<uint16_t>(bootlogo_75x75.width),
                         static_cast<uint16_t>(bootlogo_75x75.height), (int32_t *) (&bootlogo_75x75.pixel_data[0]));
    } else {
        lfb->placeSprite(50, 45, static_cast<uint16_t>(bootlogo_200x200.width),
                         static_cast<uint16_t>(bootlogo_200x200.height), (int32_t *) (&bootlogo_200x200.pixel_data[0]));
    }

    lfb->placeFilledRect(20, 80, 60, 2, Colors::HHU_BLUE_30);
    lfb->placeFilledCircle(20, 81, 1, Colors::HHU_BLUE_30);
    lfb->placeFilledCircle(80, 81, 1, Colors::HHU_BLUE_30);

    lfb->placeFilledRect(20, 80, normalizedPercentage, 2, Colors::HHU_BLUE);
    lfb->placeFilledCircle(20, 81, 1, Colors::HHU_BLUE);
    lfb->placeFilledCircle(static_cast<uint16_t>(20 + normalizedPercentage), 81, 1, Colors::HHU_BLUE);

    lfb->placeString(sun_font_8x16, 50, 90, currentActivity, Colors::HHU_GRAY, Colors::INVISIBLE);

    lfb->show();
}

void registerServices() {

    auto *graphicsService = new GraphicsService();
    graphicsService->setLinearFrameBuffer(lfb);
    graphicsService->setTextDriver(text);

    Kernel::registerService(GraphicsService::SERVICE_NAME, graphicsService);

    Kernel::registerService(EventBus::SERVICE_NAME, eventBus);

    Kernel::registerService(TimeService::SERVICE_NAME, new TimeService());
    Kernel::registerService(FileSystem::SERVICE_NAME, new FileSystem());
    Kernel::registerService(InputService::SERVICE_NAME, new InputService());
    Kernel::registerService(DebugService::SERVICE_NAME, new DebugService());
    Kernel::registerService(ModuleLoader::SERVICE_NAME, new ModuleLoader());
    Kernel::registerService(StorageService::SERVICE_NAME, new StorageService());
    Kernel::registerService(StdStreamService::SERVICE_NAME, new StdStreamService());
    Kernel::registerService(SoundService::SERVICE_NAME, new SoundService());

    ((StdStreamService *) Kernel::getService(StdStreamService::SERVICE_NAME))->setStdout(text);
    ((StdStreamService *) Kernel::getService(StdStreamService::SERVICE_NAME))->setStderr(text);
}

void initGraphics() {
    auto *vesa = new VesaGraphics();

	//Detect video capability
	if(vesa->isAvailable()) {
		lfb = vesa;
        text = new VesaText();
	} else {
		delete vesa;
		auto *cga = new CgaGraphics();
		if(cga->isAvailable()) {
			lfb = cga;
            text = new CgaText();
		} else {
			//No VBE and no CGA? Your machine is waaaaay to old...
			delete cga;
			Cpu::halt();
		}
	}
}

int32_t main() {
#if DEBUG_MODE
    initGraphics();
    text->init(80, 30, 32);
    stdout = text;

    text->setpos(0, 0);

    text->puts("Initializing Event Bus\n", 23, Colors::HHU_RED);
    eventBus = new EventBus();

    text->puts("Registering Services\n", 21, Colors::HHU_RED);
    registerServices();

    text->puts("Enabling Interrupts\n", 20, Colors::HHU_RED);
    InputService *inputService = (InputService*)Kernel::getService(InputService::SERVICE_NAME);
    inputService->getKeyboard()->plugin();
    inputService->getMouse()->plugin();

    Pit::getInstance()->plugin();
    Pit::getInstance()->setCursor(true);
    Pic::getInstance()->allow(2);

    Rtc *rtc = ((TimeService*) Kernel::getService(TimeService::SERVICE_NAME))->getRTC();
    rtc->plugin();

    text->puts("Initializing PCI Devices\n", 25, Colors::HHU_RED);
    Pci::scan();

    text->puts("Initializing Filesystem\n", 24, Colors::HHU_RED);
    FileSystem *fs = (FileSystem*) Kernel::getService(FileSystem::SERVICE_NAME);
    fs->init();
    printfUpdateStdout();

    text->puts("Loading Kernel Symbols\n", 23, Colors::HHU_RED);
    KernelSymbols::initialize();

    text->puts("Starting Threads\n", 17, Colors::HHU_RED);
    idleThread = new IdleThread();
    app = new Application();

    idleThread->start();
    eventBus->start();
    app->start();

    text->puts("\n\nFinished Booting! Please press Enter!\n", 40, Colors::HHU_BLUE);
    while(!inputService->getKeyboard()->isKeyPressed(28));

    text->clear();
#else
    initGraphics();
    lfb->init(640, 400, 32);
    lfb->enableDoubleBuffering();

    updateBootScreen(0, "Initializing Event Bus");
    eventBus = new EventBus();

    updateBootScreen(14, "Registering Services");
    registerServices();

    updateBootScreen(28, "Enabling Interrupts");
    InputService *inputService = (InputService*)Kernel::getService(InputService::SERVICE_NAME);
    inputService->getKeyboard()->plugin();
    inputService->getMouse()->plugin();

    Pit::getInstance()->plugin();
    Pit::getInstance()->setCursor(true);
    Pic::getInstance()->allow(2);

    Rtc *rtc = ((TimeService*) Kernel::getService(TimeService::SERVICE_NAME))->getRTC();
    rtc->plugin();

    updateBootScreen(42, "Initializing PCI Devices");
    Pci::scan();

    updateBootScreen(56, "Initializing Filesystem");
    FileSystem *fs = (FileSystem*) Kernel::getService(FileSystem::SERVICE_NAME);
    fs->init();
    printfUpdateStdout();

    updateBootScreen(70, "Loading Kernel Symbols");
    KernelSymbols::initialize();

    updateBootScreen(85, "Starting Threads");
    idleThread = new IdleThread();
    app = new Application();

    idleThread->start();
    eventBus->start();
    app->start();
	
    updateBootScreen(100, "Finished Booting!");
    ((TimeService*) Kernel::getService(TimeService::SERVICE_NAME))->msleep(1000);

    lfb->disableDoubleBuffering();
    lfb->clear();
#endif

    Scheduler::getInstance()->schedule();

    return 0;
}