#ifndef USER_MODEL_HPP
#define USER_MODEL_HPP

#include <nlohmann/json.hpp>
#include <bsoncxx/oid.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;

struct User {
    bsoncxx::oid id;
    std::string name;
    std::string username;
    std::string email;
    std::string password;
    std::string bio;
    std::string profilePicture;
    std::string bannerPicture;
    std::vector<bsoncxx::oid> followers;
    std::vector<bsoncxx::oid> following;
    std::vector<bsoncxx::oid> followRequests;
    std::vector<bsoncxx::oid> bookmarks;
    bool isPrivate;

    json toJson() const {
        json j = {
            {"_id", id.to_string()},
            {"name", name},
            {"username", username},
            {"email", email},
            {"profilePicture", profilePicture},
            {"bannerPicture", bannerPicture},
            {"followers", json::array()},
            {"following", json::array()},
            {"followRequests", json::array()},
            {"bookmarks", json::array()},
            {"isPrivate", isPrivate},
            {"bio", bio}
        };
        for (const auto& f : followers) j["followers"].push_back(f.to_string());
        for (const auto& f : following) j["following"].push_back(f.to_string());
        for (const auto& fr : followRequests) j["followRequests"].push_back(fr.to_string());
        for (const auto& b : bookmarks) j["bookmarks"].push_back(b.to_string());
        return j;
    }
};

#endif