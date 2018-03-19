#include <kernel/Kernel.h>
#include <devices/graphics/text/TextDriver.h>
#include "GraphicsDeviceNameNode.h"

GraphicsDeviceNameNode::GraphicsDeviceNameNode(uint8_t mode) : VirtualNode("device", REGULAR_FILE), mode(mode) {
    graphicsService = (GraphicsService *) Kernel::getService(GraphicsService::SERVICE_NAME);
}

uint64_t GraphicsDeviceNameNode::getLength() {
    switch(mode) {
        case TEXT :
            return graphicsService->getTextDriver()->getDeviceName().length();
        case LINEAR_FRAME_BUFFER :
            return graphicsService->getLinearFrameBuffer()->getDeviceName().length();
        default:
            return 0;
    }
}

char *GraphicsDeviceNameNode::readData(char *buf, uint64_t pos, uint32_t numBytes) {
    String name;

    switch(mode) {
        case TEXT :
            name = graphicsService->getTextDriver()->getDeviceName();
            break;
        case LINEAR_FRAME_BUFFER :
            name = graphicsService->getLinearFrameBuffer()->getDeviceName();
            break;
        default:
            break;
    }

    uint64_t length = name.length();

    if (pos + numBytes > length) {
        numBytes = (uint32_t) (length - pos);
    }

    memcpy(buf, (char*) name + pos, numBytes);

    return buf;
}

int32_t GraphicsDeviceNameNode::writeData(char *buf, uint64_t pos, uint32_t numBytes) {
    return -1;
}