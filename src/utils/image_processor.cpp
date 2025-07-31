#include "../../include/utils/image_processor.hpp"
#include <vips/vips8>

std::string processImage(const std::string& imageData) {
    try {
        vips::VImage image = vips::VImage::new_from_buffer(imageData.data(), imageData.size(), "");
        image = image.resize(800.0 / image.width());
        auto buffer = image.write_to_buffer(".jpg", vips::VImage::option()->set("Q", 80));
        return "data:image/jpeg;base64," + std::string(buffer.begin(), buffer.end());
    } catch (const std::exception& e) {
        return "";
    }
}