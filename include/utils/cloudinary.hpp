#ifndef CLOUDINARY_HPP
#define CLOUDINARY_HPP

#include <string>

std::string uploadToCloudinary(const std::string& imageData, const std::string& cloudName, const std::string& apiKey, const std::string& apiSecret);

#endif