#ifndef NOTIFICATION_MODEL_HPP
#define NOTIFICATION_MODEL_HPP

#include <nlohmann/json.hpp>
#include <bsoncxx/oid.hpp>
#include <string>

using json = nlohmann::json;

struct Notification {
    bsoncxx::oid id;
    bsoncxx::oid fromUser;
    bsoncxx::oid toUser;
    std::string type;
    std::string status;

    json toJson() const {
        return {
            {"_id", id.to_string()},
            {"fromUser", fromUser.to_string()},
            {"toUser", toUser.to_string()},
            {"type", type},
            {"status", status}
        };
    }
};

#endif