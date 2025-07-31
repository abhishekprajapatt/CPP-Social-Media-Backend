#include "../../include/controllers/notification_controller.hpp"
#include "../../include/models/notification_model.hpp"
#include "../../include/models/user_model.hpp"
#include <nlohmann/json.hpp>
#include <mongocxx/collection.hpp>

using json = nlohmann::json;

crow::response createFollowRequest(Database& db, const crow::request& req) {
    try {
        auto body = json::parse(req.body);
        std::string fromUserId = body["id"];
        std::string toUserId = req.url_params.get("id");

        auto users = db.getDB()["users"];
        auto toUser = users.find_one(bsoncxx::builder::stream::document{} << "_id" << bsoncxx::oid(toUserId) << bsoncxx::builder::stream::finalize);
        if (!toUser) {
            return crow::response(404, R"({"message": "User not found", "success": false})");
        }

        auto followers = toUser->view()["followers"].get_array().value;
        for (const auto& f : followers) {
            if (f.get_oid().value.to_string() == fromUserId) {
                return crow::response(400, R"({"message": "Already following this user", "success": false})");
            }
        }

        auto notifications = db.getDB()["notifications"];
        auto result = notifications.insert_one(bsoncxx::builder::stream::document{}
            << "fromUser" << bsoncxx::oid(fromUserId)
            << "toUser" << bsoncxx::oid(toUserId)
            << "type" << "follow_request"
            << "status" << "pending"
            << bsoncxx::builder::stream::finalize);

        users.update_one(
            bsoncxx::builder::stream::document{} << "_id" << bsoncxx::oid(toUserId) << bsoncxx::builder::stream::finalize,
            bsoncxx::builder::stream::document{} << "$push" << bsoncxx::builder::stream::open_document
                << "followRequests" << result->inserted_id().get_oid().value
                << bsoncxx::builder::stream::close_document << bsoncxx::builder::stream::finalize
        );

        json response = {
            {"message", "Follow request sent"},
            {"success", true},
            {"notification", {
                {"_id", result->inserted_id().get_oid().value.to_string()},
                {"fromUser", fromUserId},
                {"toUser", toUserId},
                {"type", "follow_request"},
                {"status", "pending"}
            }}
        };
        return crow::response(200, response.dump());
    } catch (const std::exception& e) {
        return crow::response(500, R"({"message": "Server Error", "success": false})");
    }
}

crow::response respondToFollowRequest(Database& db, const crow::request& req) {
    try {
        std::string notificationId = req.url_params.get("id");
        auto body = json::parse(req.body);
        bool accept = body["accept"];

        auto notifications = db.getDB()["notifications"];
        auto notification = notifications.find_one(bsoncxx::builder::stream::document{} << "_id" << bsoncxx::oid(notificationId) << bsoncxx::builder::stream::finalize);
        if (!notification) {
            return crow::response(404, R"({"message": "Notification not found", "success": false})");
        }

        auto users = db.getDB()["users"];
        auto fromUser = users.find_one(bsoncxx::builder::stream::document{} << "_id" << notification->view()["fromUser"].get_oid().value << bsoncxx::builder::stream::finalize);
        auto toUser = users.find_one(bsoncxx::builder::stream::document{} << "_id" << notification->view()["toUser"].get_oid().value << bsoncxx::builder::stream::finalize);

        if (accept) {
            users.update_one(
                bsoncxx::builder::stream::document{} << "_id" << notification->view()["toUser"].get_oid().value << bsoncxx::builder::stream::finalize,
                bsoncxx::builder::stream::document{} << "$push" << bsoncxx::builder::stream::open_document
                    << "followers" << notification->view()["fromUser"].get_oid().value
                    << bsoncxx::builder::stream::close_document
                    << "$pull" << bsoncxx::builder::stream::open_document
                    << "followRequests" << bsoncxx::oid(notificationId)
                    << bsoncxx::builder::stream::close_document << bsoncxx::builder::stream::finalize
            );
            users.update_one(
                bsoncxx::builder::stream::document{} << "_id" << notification->view()["fromUser"].get_oid().value << bsoncxx::builder::stream::finalize,
                bsoncxx::builder::stream::document{} << "$push" << bsoncxx::builder::stream::open_document
                    << "following" << notification->view()["toUser"].get_oid().value
                    << bsoncxx::builder::stream::close_document << bsoncxx::builder::stream::finalize
            );
            notifications.update_one(
                bsoncxx::builder::stream::document{} << "_id" << bsoncxx::oid(notificationId) << bsoncxx::builder::stream::finalize,
                bsoncxx::builder::stream::document{} << "$set" << bsoncxx::builder::stream::open_document
                    << "status" << "accepted"
                    << bsoncxx::builder::stream::close_document << bsoncxx::builder::stream::finalize
            );
            return crow::response(200, R"({"message": "Follow request accepted", "success": true})");
        } else {
            notifications.update_one(
                bsoncxx::builder::stream::document{} << "_id" << bsoncxx::oid(notificationId) << bsoncxx::builder::stream::finalize,
                bsoncxx::builder::stream::document{} << "$set" << bsoncxx::builder::stream::open_document
                    << "status" << "rejected"
                    << bsoncxx::builder::stream::close_document << bsoncxx::builder::stream::finalize
            );
            users.update_one(
                bsoncxx::builder::stream::document{} << "_id" << notification->view()["toUser"].get_oid().value << bsoncxx::builder::stream::finalize,
                bsoncxx::builder::stream::document{} << "$pull" << bsoncxx::builder::stream::open_document
                    << "followRequests" << bsoncxx::oid(notificationId)
                    << bsoncxx::builder::stream::close_document << bsoncxx::builder::stream::finalize
            );
            return crow::response(200, R"({"message": "Follow request rejected", "success": true})");
        }
    } catch (const std::exception& e) {
        return crow::response(500, R"({"message": "Server Error", "success": false})");
    }
}

crow::response getNotifications(Database& db, const crow::request& req) {
    try {
        std::string userId = req.url_params.get("id");
        auto notifications = db.getDB()["notifications"];
        auto cursor = notifications.find(bsoncxx::builder::stream::document{}
            << "toUser" << bsoncxx::oid(userId)
            << "status" << "pending"
            << bsoncxx::builder::stream::finalize);

        json notificationsJson = json::array();
        auto users = db.getDB()["users"];
        for (const auto& doc : cursor) {
            auto fromUser = users.find_one(bsoncxx::builder::stream::document{} << "_id" << doc["fromUser"].get_oid().value << bsoncxx::builder::stream::finalize);
            notificationsJson.push_back(json{
                {"_id", doc["_id"].get_oid().value.to_string()},
                {"fromUser", {
                    {"name", fromUser->view()["name"].get_utf8().value.to_string()},
                    {"username", fromUser->view()["username"].get_utf8().value.to_string()},
                    {"profilePicture", fromUser->view()["profilePicture"].get_utf8().value.to_string()}
                }},
                {"toUser", doc["toUser"].get_oid().value.to_string()},
                {"type", doc["type"].get_utf8().value.to_string()},
                {"status", doc["status"].get_utf8().value.to_string()}
            });
        }

        return crow::response(200, json{{"notifications", notificationsJson}, {"message", "Notifications retrieved successfully"}, {"success", true}}.dump());
    } catch (const std::exception& e) {
        return crow::response(500, R"({"message": "Server Error", "success": false})");
    }
}