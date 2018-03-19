#include <kernel/Kernel.h>
#include <devices/graphics/text/TextDriver.h>
#include "GraphicsMemoryNode.h"

GraphicsMemoryNode::GraphicsMemoryNode(uint8_t mode) : VirtualNode("memory", REGULAR_FILE), mode(mode) {
    graphicsService = (GraphicsService *) Kernel::getService(GraphicsService::SERVICE_NAME);
}

uint64_t GraphicsMemoryNode::getLength() {
    switch(mode) {
        case TEXT :
            return String::valueOf(graphicsService->getTextDriver()->getVideoMemorySize(), 10).length();
        case LINEAR_FRAME_BUFFER :
            return String::valueOf(graphicsService->getLinearFrameBuffer()->getVideoMemorySize(), 10).length();
        default:
            return 0;
    }
}

char *GraphicsMemoryNode::readData(char *buf, uint64_t pos, uint32_t numBytes) {
    String string;

    switch(mode) {
        case TEXT :
            string = String::valueOf(graphicsService->getTextDriver()->getVideoMemorySize(), 10);
            break;
        case LINEAR_FRAME_BUFFER :
            string = String::valueOf(graphicsService->getLinearFrameBuffer()->getVideoMemorySize(), 10);
            break;
        default:
            break;
    }

    uint64_t length = string.length();

    if (pos + numBytes > length) {
        numBytes = (uint32_t) (length - pos);
    }

    memcpy(buf, (char*) string + pos, numBytes);

    return buf;
}

int32_t GraphicsMemoryNode::writeData(char *buf, uint64_t pos, uint32_t numBytes) {
    return -1;
}