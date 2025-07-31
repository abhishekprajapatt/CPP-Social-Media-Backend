#include "../../include/controllers/user_controller.hpp"
#include "../../include/models/user_model.hpp"
#include <nlohmann/json.hpp>
#include <bcrypt/bcrypt.h>
#include <jwt-cpp/jwt.h>
#include <mongocxx/collection.hpp>
#include <regex>

using json = nlohmann::json;

crow::response registerUser(Database& db, const crow::request& req) {
    try {
        auto body = json::parse(req.body);
        std::string name = body["name"];
        std::string username = body["username"];
        std::string email = body["email"];
        std::string password = body["password"];
        if (name.empty() || username.empty() || email.empty() || password.empty()) {
            return crow::response(401, R"({"message": "All fields are required", "success": false})");
        }

        auto coll = db.getDB()["users"];
        auto result = coll.find_one(bsoncxx::builder::stream::document{} << "email" << email << bsoncxx::builder::stream::finalize);
        if (result) {
            return crow::response(401, R"({"message": "User already exists", "success": false})");
        }

        std::string hash = bcrypt::generateHash(password);
        coll.insert_one(bsoncxx::builder::stream::document{}
            << "name" << name
            << "username" << username
            << "email" << email
            << "password" << hash
            << "bio" << ""
            << "profilePicture" << "https://avatar.iran.liara.run/public"
            << "bannerPicture" << ""
            << "followers" << bsoncxx::builder::stream::open_array << bsoncxx::builder::stream::close_array
            << "following" << bsoncxx::builder::stream::open_array << bsoncxx::builder::stream::close_array
            << "followRequests" << bsoncxx::builder::stream::open_array << bsoncxx::builder::stream::close_array
            << "bookmarks" << bsoncxx::builder::stream::open_array << bsoncxx::builder::stream::close_array
            << "isPrivate" << false
            << bsoncxx::builder::stream::finalize);

        return crow::response(200, R"({"message": "Account created successfully", "success": true})");
    } catch (const std::exception& e) {
        return crow::response(500, R"({"message": "Server Error", "success": false})");
    }
}

crow::response loginUser(Database& db, const crow::request& req) {
    try {
        auto body = json::parse(req.body);
        std::string email = body["email"];
        std::string password = body["password"];
        if (email.empty() || password.empty()) {
            return crow::response(400, R"({"message": "All fields are required", "success": false})");
        }

        auto coll = db.getDB()["users"];
        auto result = coll.find_one(bsoncxx::builder::stream::document{} << "email" << email << bsoncxx::builder::stream::finalize);
        if (!result) {
            return crow::response(401, R"({"message": "Incorrect email", "success": false})");
        }

        auto user = result->view();
        if (!bcrypt::validatePassword(password, user["password"].get_utf8().value.to_string())) {
            return crow::response(400, R"({"message": "Incorrect password", "success": false})");
        }

        auto token = jwt::create()
            .set_issuer("your_app")
            .set_payload_claim("userId", jwt::claim(user["_id"].get_oid().value.to_string()))
            .set_expires_at(std::chrono::system_clock::now() + std::chrono::hours(24))
            .sign(jwt::algorithm::hs256{"tqeetkkdltweetkkd94kkfj43kkfkllwel"});

        json response = {
            {"message", "Login successful"},
            {"success", true},
            {"user", {
                {"_id", user["_id"].get_oid().value.to_string()},
                {"name", user["name"].get_utf8().value.to_string()},
                {"username", user["username"].get_utf8().value.to_string()},
                {"email", user["email"].get_utf8().value.to_string()},
                {"profilePicture", user["profilePicture"].get_utf8().value.to_string()},
                {"bannerPicture", user["bannerPicture"].get_utf8().value.to_string()},
                {"followers", json::array()},
                {"following", json::array()},
                {"isPrivate", user["isPrivate"].get_bool().value},
                {"bio", user["bio"].get_utf8().value.to_string()}
            }}
        };
        return crow::response(201, response.dump(), {{"Set-Cookie", "token=" + token + "; HttpOnly; Max-Age=86400"}});
    } catch (const std::exception& e) {
        return crow::response(500, R"({"message": "Server Error", "success": false})");
    }
}

crow::response logoutUser(const crow::request& req) {
    try {
        return crow::response(200, R"({"message": "Logout successful", "success": true})", {{"Set-Cookie", "token=; HttpOnly; Max-Age=0"}});
    } catch (const std::exception& e) {
        return crow::response(500, R"({"message": "Server Error", "success": false})");
    }
}

crow::response bookmarks(Database& db, const crow::request& req) {
    try {
        std::string loginUserId = req.get_header_value("userId");
        std::string tweetId = req.url_params.get("id");
        auto coll = db.getDB()["users"];
        auto user = coll.find_one(bsoncxx::builder::stream::document{} << "_id" << bsoncxx::oid(loginUserId) << bsoncxx::builder::stream::finalize);
        if (!user) {
            return crow::response(404, R"({"message": "User not found", "success": false})");
        }

        auto bookmarks = user->view()["bookmarks"].get_array().value;
        bool isBookmarked = false;
        for (const auto& b : bookmarks) {
            if (b.get_oid().value.to_string() == tweetId) {
                isBookmarked = true;
                break;
            }
        }

        if (isBookmarked) {
            coll.update_one(
                bsoncxx::builder::stream::document{} << "_id" << bsoncxx::oid(loginUserId) << bsoncxx::builder::stream::finalize,
                bsoncxx::builder::stream::document{} << "$pull" << bsoncxx::builder::stream::open_document
                    << "bookmarks" << bsoncxx::oid(tweetId)
                    << bsoncxx::builder::stream::close_document << bsoncxx::builder::stream::finalize
            );
            return crow::response(200, R"({"message": "Bookmark removed", "success": true})");
        } else {
            coll.update_one(
                bsoncxx::builder::stream::document{} << "_id" << bsoncxx::oid(loginUserId) << bsoncxx::builder::stream::finalize,
                bsoncxx::builder::stream::document{} << "$push" << bsoncxx::builder::stream::open_document
                    << "bookmarks" << bsoncxx::oid(tweetId)
                    << bsoncxx::builder::stream::close_document << bsoncxx::builder::stream::finalize
            );
            return crow::response(200, R"({"message": "Bookmark saved", "success": true})");
        }
    } catch (const std::exception& e) {
        return crow::response(500, R"({"message": "Server Error", "success": false})");
    }
}

crow::response editProfile(Database& db, const crow::request& req) {
    try {
        std::string userId = req.url_params.get("userId");
        auto body = json::parse(req.body);
        auto coll = db.getDB()["users"];
        auto user = coll.find_one(bsoncxx::builder::stream::document{} << "_id" << bsoncxx::oid(userId) << bsoncxx::builder::stream::finalize);
        if (!user) {
            return crow::response(404, R"({"message": "User not found", "success": false})");
        }

        bsoncxx::builder::stream::document update;
        update << "$set" << bsoncxx::builder::stream::open_document;
        if (body.contains("name")) update << "name" << body["name"].get<std::string>();
        if (body.contains("username")) update << "username" << body["username"].get<std::string>();
        if (body.contains("bio")) update << "bio" << body["bio"].get<std::string>();
        if (body.contains("profilePicture")) update << "profilePicture" << body["profilePicture"].get<std::string>();
        if (body.contains("bannerPicture")) update << "bannerPicture" << body["bannerPicture"].get<std::string>();
        if (body.contains("isPrivate")) update << "isPrivate" << body["isPrivate"].get<bool>();
        update << bsoncxx::builder::stream::close_document;

        coll.update_one(
            bsoncxx::builder::stream::document{} << "_id" << bsoncxx::oid(userId) << bsoncxx::builder::stream::finalize,
            update
        );

        auto updatedUser = coll.find_one(bsoncxx::builder::stream::document{} << "_id" << bsoncxx::oid(userId) << bsoncxx::builder::stream::finalize);
        json response = {
            {"message", "Profile updated successfully"},
            {"success", true},
            {"user", User{
                updatedUser->view()["_id"].get_oid().value,
                updatedUser->view()["name"].get_utf8().value.to_string(),
                updatedUser->view()["username"].get_utf8().value.to_string(),
                updatedUser->view()["email"].get_utf8().value.to_string(),
                "",
                updatedUser->view()["bio"].get_utf8().value.to_string(),
                updatedUser->view()["profilePicture"].get_utf8().value.to_string(),
                updatedUser->view()["bannerPicture"].get_utf8().value.to_string(),
                {}, {}, {}, {},
                updatedUser->view()["isPrivate"].get_bool().value
            }.toJson()}
        };
        return crow::response(200, response.dump());
    } catch (const std::exception& e) {
        return crow::response(500, R"({"message": "Server Error", "success": false})");
    }
}

crow::response getProfile(Database& db, const crow::request& req) {
    try {
        std::string id = req.url_params.get("id");
        auto coll = db.getDB()["users"];
        auto user = coll.find_one(bsoncxx::builder::stream::document{} << "_id" << bsoncxx::oid(id) << bsoncxx::builder::stream::finalize);
        if (!user) {
            return crow::response(401, R"({"message": "User not found", "success": false})");
        }

        json response = {
            {"success", true},
            {"user", User{
                user->view()["_id"].get_oid().value,
                user->view()["name"].get_utf8().value.to_string(),
                user->view()["username"].get_utf8().value.to_string(),
                user->view()["email"].get_utf8().value.to_string(),
                "",
                user->view()["bio"].get_utf8().value.to_string(),
                user->view()["profilePicture"].get_utf8().value.to_string(),
                user->view()["bannerPicture"].get_utf8().value.to_string(),
                {}, {}, {}, {},
                user->view()["isPrivate"].get_bool().value
            }.toJson()}
        };
        return crow::response(200, response.dump());
    } catch (const std::exception& e) {
        return crow::response(500, R"({"message": "Server Error", "success": false})");
    }
}

crow::response getOthersUsers(Database& db, const crow::request& req) {
    try {
        std::string id = req.url_params.get("id");
        auto coll = db.getDB()["users"];
        auto cursor = coll.find(bsoncxx::builder::stream::document{} << "_id" << bsoncxx::builder::stream::open_document
            << "$ne" << bsoncxx::oid(id) << bsoncxx::builder::stream::close_document << bsoncxx::builder::stream::finalize);

        json users = json::array();
        for (const auto& doc : cursor) {
            users.push_back(json{
                {"_id", doc["_id"].get_oid().value.to_string()},
                {"name", doc["name"].get_utf8().value.to_string()},
                {"username", doc["username"].get_utf8().value.to_string()},
                {"profilePicture", doc["profilePicture"].get_utf8().value.to_string()},
                {"followers", json::array()},
                {"following", json::array()},
                {"isPrivate", doc["isPrivate"].get_bool().value}
            });
        }

        if (users.empty()) {
            return crow::response(401, R"({"message": "No other users found", "success": false})");
        }

        return crow::response(200, json{{"otherUsers", users}, {"success", true}}.dump());
    } catch (const std::exception& e) {
        return crow::response(500, R"({"message": "Server Error", "success": false})");
    }
}

crow::response followUser(Database& db, const crow::request& req) {
    try {
        std::string loginUserId = req.get_header_value("userId");
        std::string otherUserId = req.url_params.get("id");
        auto coll = db.getDB()["users"];
        auto loginUser = coll.find_one(bsoncxx::builder::stream::document{} << "_id" << bsoncxx::oid(loginUserId) << bsoncxx::builder::stream::finalize);
        auto otherUser = coll.find_one(bsoncxx::builder::stream::document{} << "_id" << bsoncxx::oid(otherUserId) << bsoncxx::builder::stream::finalize);
        if (!otherUser) {
            return crow::response(404, R"({"message": "User not found", "success": false})");
        }

        auto followers = otherUser->view()["followers"].get_array().value;
        for (const auto& f : followers) {
            if (f.get_oid().value.to_string() == loginUserId) {
                return crow::response(400, json{{"message", "Already following " + otherUser->view()["name"].get_utf8().value.to_string()}, {"success", false}}.dump());
            }
        }

        if (otherUser->view()["isPrivate"].get_bool().value) {
            return crow::response(403, R"({"message": "Private profile, follow request required", "success": false})");
        }

        coll.update_one(
            bsoncxx::builder::stream::document{} << "_id" << bsoncxx::oid(otherUserId) << bsoncxx::builder::stream::finalize,
            bsoncxx::builder::stream::document{} << "$push" << bsoncxx::builder::stream::open_document
                << "followers" << bsoncxx::oid(loginUserId)
                << bsoncxx::builder::stream::close_document << bsoncxx::builder::stream::finalize
        );
        coll.update_one(
            bsoncxx::builder::stream::document{} << "_id" << bsoncxx::oid(loginUserId) << bsoncxx::builder::stream::finalize,
            bsoncxx::builder::stream::document{} << "$push" << bsoncxx::builder::stream::open_document
                << "following" << bsoncxx::oid(otherUserId)
                << bsoncxx::builder::stream::close_document << bsoncxx::builder::stream::finalize
        );

        return crow::response(200, json{{"message", loginUser->view()["name"].get_utf8().value.to_string() + " followed " + otherUser->view()["name"].get_utf8().value.to_string()}, {"success", true}}.dump());
    } catch (const std::exception& e) {
        return crow::response(500, R"({"message": "Server Error", "success": false})");
    }
}

crow::response unfollowUser(Database& db, const crow::request& req) {
    try {
        std::string loginUserId = req.get_header_value("userId");
        std::string otherUserId = req.url_params.get("id");
        auto coll = db.getDB()["users"];
        auto loginUser = coll.find_one(bsoncxx::builder::stream::document{} << "_id" << bsoncxx::oid(loginUserId) << bsoncxx::builder::stream::finalize);
        auto otherUser = coll.find_one(bsoncxx::builder::stream::document{} << "_id" << bsoncxx::oid(otherUserId) << bsoncxx::builder::stream::finalize);
        if (!otherUser) {
            return crow::response(404, R"({"message": "User not found", "success": false})");
        }

        auto following = loginUser->view()["following"].get_array().value;
        bool isFollowing = false;
        for (const auto& f : following) {
            if (f.get_oid().value.to_string() == otherUserId) {
                isFollowing = true;
                break;
            }
        }

        if (!isFollowing) {
            return crow::response(400, json{{"message", "Not following " + otherUser->view()["name"].get_utf8().value.to_string()}, {"success", false}}.dump());
        }

        coll.update_one(
            bsoncxx::builder::stream::document{} << "_id" << bsoncxx::oid(otherUserId) << bsoncxx::builder::stream::finalize,
            bsoncxx::builder::stream::document{} << "$pull" << bsoncxx::builder::stream::open_document
                << "followers" << bsoncxx::oid(loginUserId)
                << bsoncxx::builder::stream::close_document << bsoncxx::builder::stream::finalize
        );
        coll.update_one(
            bsoncxx::builder::stream::document{} << "_id" << bsoncxx::oid(loginUserId) << bsoncxx::builder::stream::finalize,
            bsoncxx::builder::stream::document{} << "$pull" << bsoncxx::builder::stream::open_document
                << "following" << bsoncxx::oid(otherUserId)
                << bsoncxx::builder::stream::close_document << bsoncxx::builder::stream::finalize
        );

        return crow::response(200, json{{"message", loginUser->view()["name"].get_utf8().value.to_string() + " unfollowed " + otherUser->view()["name"].get_utf8().value.to_string()}, {"success", true}}.dump());
    } catch (const std::exception& e) {
        return crow::response(500, R"({"message": "Server Error", "success": false})");
    }
}

crow::response searchUsers(Database& db, const crow::request& req) {
    try {
        std::string query = req.url_params.get("query");
        if (query.empty()) {
            return crow::response(400, R"({"message": "Search query is required", "success": false})");
        }

        auto coll = db.getDB()["users"];
        std::regex regex(query, std::regex_constants::icase);
        auto cursor = coll.find(bsoncxx::builder::stream::document{}
            << "$or" << bsoncxx::builder::stream::open_array
                << bsoncxx::builder::stream::open_document << "name" << bsoncxx::builder::stream::open_document << "$regex" << query << "$options" << "i" << bsoncxx::builder::stream::close_document << bsoncxx::builder::stream::close_document
                << bsoncxx::builder::stream::open_document << "username" << bsoncxx::builder::stream::open_document << "$regex" << query << "$options" << "i" << bsoncxx::builder::stream::close_document << bsoncxx::builder::stream::close_document
            << bsoncxx::builder::stream::close_array << bsoncxx::builder::stream::finalize);

        json users = json::array();
        for (const auto& doc : cursor) {
            users.push_back(json{
                {"_id", doc["_id"].get_oid().value.to_string()},
                {"name", doc["name"].get_utf8().value.to_string()},
                {"username", doc["username"].get_utf8().value.to_string()},
                {"profilePicture", doc["profilePicture"].get_utf8().value.to_string()},
                {"isPrivate", doc["isPrivate"].get_bool().value}
            });
        }

        return crow::response(200, json{{"users", users}, {"success", true}}.dump());
    } catch (const std::exception& e) {
        return crow::response(500, R"({"message": "Server Error", "success": false})");
    }
}