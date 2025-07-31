#include "../../include/utils/cloudinary.hpp"
#include <httplib.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::string uploadToCloudinary(const std::string& imageData, const std::string& cloudName, const std::string& apiKey, const std::string& apiSecret) {
    httplib::Client cli("https://api.cloudinary.com");
    httplib::MultipartFormDataItems items = {
        {"file", imageData, "image.jpg", "image/jpeg"},
        {"upload_preset", "your_upload_preset"}, // Replace with your Cloudinary preset
        {"cloud_name", cloudName},
        {"api_key", apiKey},
        {"api_secret", apiSecret}
    };
    auto res = cli.Post("/v1_1/" + cloudName + "/image/upload", items);
    if (res && res->status == 200) {
        return json::parse(res->body)["secure_url"];
    }
    return "";
}