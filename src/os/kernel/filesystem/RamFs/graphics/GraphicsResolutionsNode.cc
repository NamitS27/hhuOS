#include <kernel/Kernel.h>
#include <devices/graphics/text/TextDriver.h>
#include "GraphicsResolutionsNode.h"

GraphicsResolutionsNode::GraphicsResolutionsNode(uint8_t mode) : VirtualNode("resolutions", REGULAR_FILE), mode(mode) {
    graphicsService = (GraphicsService *) Kernel::getService(GraphicsService::SERVICE_NAME);
}

uint64_t GraphicsResolutionsNode::getLength() {
    String string;

    switch(mode) {
        case TEXT : {
            Util::ArrayList<TextDriver::TextResolution> &textResolutions = graphicsService->getTextDriver()->getTextResolutions();

            for (const TextDriver::TextResolution &currentRes : textResolutions) {
                string += String::valueOf(currentRes.columns, 10) + String(" x ") +
                          String::valueOf(currentRes.rows, 10) + String(" @ ") +
                          String::valueOf(currentRes.depth, 10) + String("\n");
            }

            break;
        }
        case LINEAR_FRAME_BUFFER : {
            Util::ArrayList<LinearFrameBuffer::LfbResolution> &lfbResolutions = graphicsService->getLinearFrameBuffer()->getLfbResolutions();

            for (const LinearFrameBuffer::LfbResolution &currentRes : lfbResolutions) {
                string += String::valueOf(currentRes.resX, 10) + String(" x ") +
                          String::valueOf(currentRes.resY, 10) + String(" @ ") +
                          String::valueOf(currentRes.depth, 10) + String("\n");
            }

            break;
        }
        default:
            break;
    }

    return string.length();
}

char *GraphicsResolutionsNode::readData(char *buf, uint64_t pos, uint32_t numBytes) {
    String string;

    switch(mode) {
        case TEXT : {
            Util::ArrayList<TextDriver::TextResolution> &textResolutions = graphicsService->getTextDriver()->getTextResolutions();

            for (const TextDriver::TextResolution &currentRes : textResolutions) {
                string += String::valueOf(currentRes.columns, 10) + String(" x ") +
                          String::valueOf(currentRes.rows, 10) + String(" @ ") +
                          String::valueOf(currentRes.depth, 10) + String("\n");
            }

            break;
        }
        case LINEAR_FRAME_BUFFER : {
            Util::ArrayList<LinearFrameBuffer::LfbResolution> &lfbResolutions = graphicsService->getLinearFrameBuffer()->getLfbResolutions();

            for (const LinearFrameBuffer::LfbResolution &currentRes : lfbResolutions) {
                string += String::valueOf(currentRes.resX, 10) + String(" x ") +
                          String::valueOf(currentRes.resY, 10) + String(" @ ") +
                          String::valueOf(currentRes.depth, 10) + String("\n");
            }

            break;
        }
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

int32_t GraphicsResolutionsNode::writeData(char *buf, uint64_t pos, uint32_t numBytes) {
    return -1;
}