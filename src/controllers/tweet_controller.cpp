#include "../../include/controllers/tweet_controller.hpp"
#include "../../include/models/tweet_model.hpp"
#include "../../include/models/user_model.hpp"
#include "../../include/utils/cloudinary.hpp"
#include "../../include/utils/image_processor.hpp"
#include <nlohmann/json.hpp>
#include <mongocxx/collection.hpp>

using json = nlohmann::json;

crow::response createTweet(Database& db, const crow::request& req) {
    try {
        auto body = json::parse(req.body);
        std::string description = body["description"];
        std::string id = body["id"];
        std::string imageData = body.value("image", "");
        if (description.empty() || id.empty()) {
            return crow::response(401, R"({"message": "Fields are required!", "success": false})");
        }

        std::string imageUrl;
        if (!imageData.empty()) {
            std::string processedImage = processImage(imageData);
            if (!processedImage.empty()) {
                imageUrl = uploadToCloudinary(processedImage, "dl2bgzrvo", "843851915742314", "XWkObXrVlumhCXUvEcQq0M2E95A");
            }
        }

        auto users = db.getDB()["users"];
        auto user = users.find_one(bsoncxx::builder::stream::document{} << "_id" << bsoncxx::oid(id) << bsoncxx::builder::stream::finalize);
        if (!user) {
            return crow::response(404, R"({"message": "User not found", "success": false})");
        }

        auto tweets = db.getDB()["tweets"];
        auto result = tweets.insert_one(bsoncxx::builder::stream::document{}
            << "description" << description
            << "image" << imageUrl
            << "like" << bsoncxx::builder::stream::open_array << bsoncxx::builder::stream::close_array
            << "comment" << bsoncxx::builder::stream::open_array << bsoncxx::builder::stream::close_array
            << "userId" << bsoncxx::oid(id)
            << "userDetails" << bsoncxx::builder::stream::open_document
                << "name" << user->view()["name"].get_utf8().value.to_string()
                << "username" << user->view()["username"].get_utf8().value.to_string()
                << "profilePicture" << user->view()["profilePicture"].get_utf8().value.to_string()
                << bsoncxx::builder::stream::close_document
            << bsoncxx::builder::stream::finalize);

        json response = {
            {"message", "Tweet created successfully"},
            {"success", true},
            {"tweet", {
                {"_id", result->inserted_id().get_oid().value.to_string()},
                {"description", description},
                {"image", imageUrl},
                {"userId", id},
                {"userDetails", {
                    {"name", user->view()["name"].get_utf8().value.to_string()},
                    {"username", user->view()["username"].get_utf8().value.to_string()},
                    {"profilePicture", user->view()["profilePicture"].get_utf8().value.to_string()}
                }}
            }}
        };
        return crow::response(200, response.dump());
    } catch (const std::exception& e) {
        return crow::response(500, R"({"message": "Server Error", "success": false})");
    }
}

crow::response deleteTweet(Database& db, const crow::request& req) {
    try {
        std::string id = req.url_params.get("id");
        auto tweets = db.getDB()["tweets"];
        auto result = tweets.find_one(bsoncxx::builder::stream::document{} << "_id" << bsoncxx::oid(id) << bsoncxx::builder::stream::finalize);
        if (!result) {
            return crow::response(404, R"({"message": "Tweet not found", "success": false})");
        }

        tweets.delete_one(bsoncxx::builder::stream::document{} << "_id" << bsoncxx::oid(id) << bsoncxx::builder::stream::finalize);
        return crow::response(200, R"({"message": "Tweet deleted successfully", "success": true})");
    } catch (const std::exception& e) {
        return crow::response(500, R"({"message": "Server Error", "success": false})");
    }
}

crow::response likeOrDislike(Database& db, const crow::request& req) {
    try {
        std::string loginUserId = req.get_header_value("userId");
        std::string tweetId = req.url_params.get("id");
        auto tweets = db.getDB()["tweets"];
        auto tweet = tweets.find_one(bsoncxx::builder::stream::document{} << "_id" << bsoncxx::oid(tweetId) << bsoncxx::builder::stream::finalize);
        if (!tweet) {
            return crow::response(404, R"({"message": "Tweet not found", "success": false})");
        }

        auto likes = tweet->view()["like"].get_array().value;
        bool isLiked = false;
        for (const auto& l : likes) {
            if (l.get_oid().value.to_string() == loginUserId) {
                isLiked = true;
                break;
            }
        }

        if (isLiked) {
            tweets.update_one(
                bsoncxx::builder::stream::document{} << "_id" << bsoncxx::oid(tweetId) << bsoncxx::builder::stream::finalize,
                bsoncxx::builder::stream::document{} << "$pull" << bsoncxx::builder::stream::open_document
                    << "like" << bsoncxx::oid(loginUserId)
                    << bsoncxx::builder::stream::close_document << bsoncxx::builder::stream::finalize
            );
            return crow::response(200, R"({"message": "Tweet disliked", "success": true})");
        } else {
            tweets.update_one(
                bsoncxx::builder::stream::document{} << "_id" << bsoncxx::oid(tweetId) << bsoncxx::builder::stream::finalize,
                bsoncxx::builder::stream::document{} << "$push" << bsoncxx::builder::stream::open_document
                    << "like" << bsoncxx::oid(loginUserId)
                    << bsoncxx::builder::stream::close_document << bsoncxx::builder::stream::finalize
            );
            return crow::response(200, R"({"message": "Tweet liked", "success": true})");
        }
    } catch (const std::exception& e) {
        return crow::response(500, R"({"message": "Server Error", "success": false})");
    }
}

crow::response commentOnTweet(Database& db, const crow::request& req) {
    try {
        auto body = json::parse(req.body);
        std::string userId = body["id"];
        std::string comment = body["comment"];
        std::string tweetId = req.url_params.get("id");
        auto tweets = db.getDB()["tweets"];
        auto tweet = tweets.find_one(bsoncxx::builder::stream::document{} << "_id" << bsoncxx::oid(tweetId) << bsoncxx::builder::stream::finalize);
        if (!tweet) {
            return crow::response(404, R"({"message": "Tweet not found", "success": false})");
        }

        auto users = db.getDB()["users"];
        auto user = users.find_one(bsoncxx::builder::stream::document{} << "_id" << bsoncxx::oid(userId) << bsoncxx::builder::stream::finalize);
        if (!user) {
            return crow::response(404, R"({"message": "User not found", "success": false})");
        }

        tweets.update_one(
            bsoncxx::builder::stream::document{} << "_id" << bsoncxx::oid(tweetId) << bsoncxx::builder::stream::finalize,
            bsoncxx::builder::stream::document{} << "$push" << bsoncxx::builder::stream::open_document
                << "comment" << bsoncxx::builder::stream::open_document
                    << "user" << bsoncxx::oid(userId)
                    << "text" << comment
                    << "userDetails" << bsoncxx::builder::stream::open_document
                        << "name" << user->view()["name"].get_utf8().value.to_string()
                        << "username" << user->view()["username"].get_utf8().value.to_string()
                        << "profilePicture" << user->view()["profilePicture"].get_utf8().value.to_string()
                        << bsoncxx::builder::stream::close_document
                    << bsoncxx::builder::stream::close_document
                << bsoncxx::builder::stream::close_document << bsoncxx::builder::stream::finalize
        );

        return crow::response(200, R"({"message": "Comment added successfully", "success": true})");
    } catch (const std::exception& e) {
        return crow::response(500, R"({"message": "Server Error", "success": false})");
    }
}

crow::response getAllTweets(Database& db, const crow::request& req) {
    try {
        std::string id = req.url_params.get("id");
        auto users = db.getDB()["users"];
        auto user = users.find_one(bsoncxx::builder::stream::document{} << "_id" << bsoncxx::oid(id) << bsoncxx::builder::stream::finalize);
        if (!user) {
            return crow::response(404, R"({"message": "User not found", "success": false})");
        }

        auto tweets = db.getDB()["tweets"];
        auto userTweets = tweets.find(bsoncxx::builder::stream::document{} << "userId" << bsoncxx::oid(id) << bsoncxx::builder::stream::finalize);
        json tweetsJson = json::array();
        for (const auto& doc : userTweets) {
            tweetsJson.push_back(Tweet{
                doc["_id"].get_oid().value,
                doc["description"].get_utf8().value.to_string(),
                doc["image"].get_utf8().value.to_string(),
                {},
                {},
                doc["userId"].get_oid().value,
                Tweet::UserDetails{
                    doc["userDetails"]["name"].get_utf8().value.to_string(),
                    doc["userDetails"]["username"].get_utf8().value.to_string(),
                    doc["userDetails"]["profilePicture"].get_utf8().value.to_string()
                }
            }.toJson());
        }

        auto following = user->view()["following"].get_array().value;
        for (const auto& f : following) {
            auto otherUser = users.find_one(bsoncxx::builder::stream::document{} << "_id" << f.get_oid().value << bsoncxx::builder::stream::finalize);
            bool isPrivate = otherUser->view()["isPrivate"].get_bool().value;
            bool isFollower = false;
            for (const auto& follower : otherUser->view()["followers"].get_array().value) {
                if (follower.get_oid().value.to_string() == id) {
                    isFollower = true;
                    break;
                }
            }
            if (!isPrivate || isFollower) {
                auto otherTweets = tweets.find(bsoncxx::builder::stream::document{} << "userId" << f.get_oid().value << bsoncxx::builder::stream::finalize);
                for (const auto& doc : otherTweets) {
                    tweetsJson.push_back(Tweet{
                        doc["_id"].get_oid().value,
                        doc["description"].get_utf8().value.to_string(),
                        doc["image"].get_utf8().value.to_string(),
                        {},
                        {},
                        doc["userId"].get_oid().value,
                        Tweet::UserDetails{
                            doc["userDetails"]["name"].get_utf8().value.to_string(),
                            doc["userDetails"]["username"].get_utf8().value.to_string(),
                            doc["userDetails"]["profilePicture"].get_utf8().value.to_string()
                        }
                    }.toJson());
                }
            }
        }

        return crow::response(200, json{{"tweets", tweetsJson}, {"message", "All tweets retrieved"}, {"success", true}}.dump());
    } catch (const std::exception& e) {
        return crow::response(500, R"({"message": "Server Error", "success": false})");
    }
}

crow::response getFollowingTweets(Database& db, const crow::request& req) {
    try {
        std::string id = req.url_params.get("id");
        auto users = db.getDB()["users"];
        auto user = users.find_one(bsoncxx::builder::stream::document{} << "_id" << bsoncxx::oid(id) << bsoncxx::builder::stream::finalize);
        if (!user) {
            return crow::response(404, R"({"message": "User not found", "success": false})");
        }

        json tweetsJson = json::array();
        auto following = user->view()["following"].get_array().value;
        auto tweets = db.getDB()["tweets"];
        for (const auto& f : following) {
            auto otherUser = users.find_one(bsoncxx::builder::stream::document{} << "_id" << f.get_oid().value << bsoncxx::builder::stream::finalize);
            bool isPrivate = otherUser->view()["isPrivate"].get_bool().value;
            bool isFollower = false;
            for (const auto& follower : otherUser->view()["followers"].get_array().value) {
                if (follower.get_oid().value.to_string() == id) {
                    isFollower = true;
                    break;
                }
            }
            if (!isPrivate || isFollower) {
                auto otherTweets = tweets.find(bsoncxx::builder::stream::document{} << "userId" << f.get_oid().value << bsoncxx::builder::stream::finalize);
                for (const auto& doc : otherTweets) {
                    tweetsJson.push_back(Tweet{
                        doc["_id"].get_oid().value,
                        doc["description"].get_utf8().value.to_string(),
                        doc["image"].get_utf8().value.to_string(),
                        {},
                        {},
                        doc["userId"].get_oid().value,
                        Tweet::UserDetails{
                            doc["userDetails"]["name"].get_utf8().value.to_string(),
                            doc["userDetails"]["username"].get_utf8().value.to_string(),
                            doc["userDetails"]["profilePicture"].get_utf8().value.to_string()
                        }
                    }.toJson());
                }
            }
        }

        return crow::response(200, json{{"tweets", tweetsJson}, {"message", "Following tweets retrieved"}, {"success", true}}.dump());
    } catch (const std::exception& e) {
        return crow::response(500, R"({"message": "Server Error", "success": false})");
    }
}