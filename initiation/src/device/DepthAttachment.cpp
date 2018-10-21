#include "device/DepthAttachment.hpp"

DepthAttachment::DepthAttachment() {
    mReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
}


