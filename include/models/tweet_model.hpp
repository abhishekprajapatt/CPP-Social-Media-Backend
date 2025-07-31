#ifndef TWEET_MODEL_HPP
#define TWEET_MODEL_HPP

#include <nlohmann/json.hpp>
#include <bsoncxx/oid.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;

struct Tweet {
    bsoncxx::oid id;
    std::string description;
    std::string image;
    std::vector<bsoncxx::oid> like;
    struct Comment {
        bsoncxx::oid user;
        std::string text;
        struct UserDetails {
            std::string name;
            std::string username;
            std::string profilePicture;
        } userDetails;
        json toJson() const {
            return {
                {"user", user.to_string()},
                {"text", text},
                {"userDetails", {
                    {"name", userDetails.name},
                    {"username", userDetails.username},
                    {"profilePicture", userDetails.profilePicture}
                }}
            };
        }
    };
    std::vector<Comment> comment;
    bsoncxx::oid userId;
    struct UserDetails {
        std::string name;
        std::string username;
        std::string profilePicture;
    } userDetails;

    json toJson() const {
        json j = {
            {"_id", id.to_string()},
            {"description", description},
            {"image", image},
            {"like", json::array()},
            {"comment", json::array()},
            {"userId", userId.to_string()},
            {"userDetails", {
                {"name", userDetails.name},
                {"username", userDetails.username},
                {"profilePicture", userDetails.profilePicture}
            }}
        };
        for (const auto& l : like) j["like"].push_back(l.to_string());
        for (const auto& c : comment) j["comment"].push_back(c.toJson());
        return j;
    }
};

#endif