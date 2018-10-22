#include "vulkan/ColorAttachment.hpp"

ColorAttachment::ColorAttachment() {
    mReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
}


