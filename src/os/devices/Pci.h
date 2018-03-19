/*****************************************************************************
 *                                                                           *
 *                                   P C I                                   *
 *                                                                           *
 *---------------------------------------------------------------------------*
 * Beschreibung:    PCI-Bus                                                  *
 *                                                                           *
 * Autor:           Filip Krakowski, 14.10.2017                              *
 *****************************************************************************/

#ifndef __PCI_include__
#define __PCI_include__

#define PCI_DEBUG 0

#define PCI_UHCI_ENABLED 0
#define PCI_EHCI_ENABLED 0
#define PCI_AHCI_ENABLED 1
#define PCI_IDE_ENABLED  0

#if PCI_DEBUG
#include "lib/libc/printf.h"
#define     PCI_TRACE(...) printf("[PCI] " __VA_ARGS__)
#else
#define     PCI_TRACE(...)
#endif

#include "kernel/IOport.h"

#include <stdint.h>
#include <kernel/services/StorageService.h>
#include <lib/util/ArrayList.h>

class Pci {

public:

    Pci() = delete;

    Pci(const Pci &copy) = delete;

    struct Device {
        uint8_t     bus;
        uint8_t     device;
        uint8_t     function;
        uint16_t    vendorId;
        uint16_t    deviceId;
        uint8_t     revision;
        uint8_t     pi;
        uint8_t     baseClass;
        uint8_t     subClass;
        uint8_t     ssid;
        uint8_t     ssvid;
        uint8_t     cap;
        uint8_t     intr;

        bool operator!=(const Device &other);
    };

    static Util::ArrayList<Pci::Device> &getDevices();

    static uint32_t  readDoubleWord(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);

    static uint16_t  readWord(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);

    static uint8_t   readByte(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);

    static void      writeDoubleWord(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t value);

    static void      writeWord(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t value);

    static void      writeByte(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t value);

    static uint16_t  getVendorId(uint8_t bus, uint8_t device, uint8_t function);

    static uint16_t  getDeviceId(uint8_t bus, uint8_t device, uint8_t function);

    static uint16_t  getCommand(uint8_t bus, uint8_t device, uint8_t function);

    static uint16_t  getStatus(uint8_t bus, uint8_t device, uint8_t function);

    static uint8_t   getRevision(uint8_t bus, uint8_t device, uint8_t function);

    static uint8_t   getClassId(uint8_t bus, uint8_t device, uint8_t function);

    static uint8_t   getSubclassId(uint8_t bus, uint8_t device, uint8_t function);

    static uint8_t   getProgrammingInterface(uint8_t bus, uint8_t device, uint8_t function);

    static uint8_t   getCacheLineSize(uint8_t bus, uint8_t device, uint8_t function);

    static uint8_t   getMasterLatencyTimer(uint8_t bus, uint8_t device, uint8_t function);

    static uint8_t   getHeaderType(uint8_t bus, uint8_t device, uint8_t function);

    static uint32_t  getAbar(uint8_t bus, uint8_t device, uint8_t function);

    static uint16_t  getSubsysId(uint8_t bus, uint8_t device, uint8_t function);

    static uint16_t  getSubsysVendorId(uint8_t bus, uint8_t device, uint8_t function);

    static uint8_t   getCapabilityPointer(uint8_t bus, uint8_t device, uint8_t function);

    static uint16_t  getInterruptInfo(uint8_t bus, uint8_t device, uint8_t function);

    static uint8_t   getSecondaryBus(uint8_t bus, uint8_t device, uint8_t function);

    static uint8_t   getInterruptLine(uint8_t bus, uint8_t device, uint8_t function);

    static void      enableBusMaster(uint8_t bus, uint8_t device, uint8_t function);

    static void      enableIoSpace(uint8_t bus, uint8_t device, uint8_t function);

    static void      enableMemorySpace(uint8_t bus, uint8_t device, uint8_t function);

    static uint8_t   findCapability(uint8_t bus, uint8_t device, uint8_t function, uint8_t capId);

    static void      scan();

    static void      printRegisters(const Device &device);

    /* Multifunction */
    static const uint8_t    MULTIFUNCTION_BIT = 0x80;

    /* Register offsets */
    static const uint8_t    PCI_HEADER_VENDOR            = 0x00;

    static const uint8_t    PCI_HEADER_DEVICE            = 0x02;

    static const uint8_t    PCI_HEADER_COMMAND           = 0x04;

    static const uint8_t    PCI_HEADER_COMMAND_IO        = 0x01;

    static const uint8_t    PCI_HEADER_COMMAND_MSE       = 0x02;

    static const uint8_t    PCI_HEADER_COMMAND_BME       = 0x04;

    static const uint8_t    PCI_HEADER_STATUS            = 0x06;

    static const uint8_t    PCI_HEADER_REVISION          = 0x08;

    static const uint8_t    PCI_HEADER_PROGIF            = 0x09;

    static const uint8_t    PCI_HEADER_SUBCLASS          = 0x0A;

    static const uint8_t    PCI_HEADER_CLASS             = 0x0B;

    static const uint8_t    PCI_HEADER_CLS               = 0x0C;

    static const uint8_t    PCI_HEADER_MLT               = 0x0D;

    static const uint8_t    PCI_HEADER_TYPE              = 0x0E;

    static const uint8_t    PCI_HEADER_BIST              = 0x0F;

    static const uint8_t    PCI_HEADER_BAR0              = 0x10;

    static const uint8_t    PCI_HEADER_BAR1              = 0x14;

    static const uint8_t    PCI_HEADER_BAR2              = 0x18;

    static const uint8_t    PCI_HEADER_BAR3              = 0x1C;

    static const uint8_t    PCI_HEADER_BAR4              = 0x20;

    static const uint8_t    PCI_HEADER_BAR5              = 0x24;

    static const uint8_t    PCI_HEADER_SSVID             = 0x2C;

    static const uint8_t    PCI_HEADER_SSID              = 0x2D;

    static const uint8_t    PCI_HEADER_CAP               = 0x34;

    static const uint8_t    PCI_HEADER_INTR              = 0x3C;

    static const uint8_t    PCI_HEADER_SECONDARY_BUS     = 0x19;

    /* Class Codes */
    static const uint8_t    CLASS_MASS_STORAGE_DEVICE    = 0x01;

    static const uint8_t    CLASS_BRIDGE_DEVICE          = 0x06;

    static const uint8_t    CLASS_SERIAL_BUS_DEVICE      = 0x0C;

    /* Subclass Codes */
    static const uint8_t    SUBCLASS_PCI_TO_PCI          = 0x04;

    static const uint8_t    SUBCLASS_IDE                 = 0x01;

    static const uint8_t    SUBCLASS_SERIAL_ATA          = 0x06;

    static const uint8_t    SUBCLASS_USB                 = 0x03;

    /* Programming Interface Codes */
    static const uint8_t    PROGIF_UHCI                  = 0x00;

    static const uint8_t    PROGIF_EHCI                  = 0x20;

    /* Capability Codes */
    static const uint8_t    CAP_POWER_MANAGEMENT         = 0x01;

    /* PCI Power Management */
    static const uint8_t    POWER_CAPABILITIES           = 0x02;

    static const uint8_t    POWER_CONTROL_STATUS         = 0x04;

    static const uint8_t    POWER_BRIDGE_EXTENSION       = 0x06;

    static const uint8_t    POWER_DATA                   = 0x07;

private:

    static StorageService *storageService;

    /* PCI IO Ports */
    static const IOport CONFIG_ADDRESS;

    static const IOport CONFIG_DATA;

    /* PCI Constants */
    static const uint8_t    NUM_DEVICES                  = 32;

    static const uint8_t    NUM_FUNCTIONS                = 8;

    static const uint16_t   INVALID_VENDOR               = 0xFFFF;

    static const uint8_t    INVALID_CAPABILITY           = 0x00;

    static Util::ArrayList<Device> pciDevices;

    static uint8_t     ahciCount;

    static void        prepareRegister(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);

    static void        checkFunction(uint8_t bus, uint8_t device, uint8_t function);

    static void        checkDevice(uint8_t bus, uint8_t device);

    static void        scanBus(uint8_t bus);

    static void        registerDevice(const Device &device);

    static Pci::Device      readDevice(uint8_t bus, uint8_t device, uint8_t function);
};

#endif