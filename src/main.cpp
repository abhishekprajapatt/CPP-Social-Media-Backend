#include <crow.h>
#include <crow/middlewares/cors.h>
#include "../include/controllers/notification_controller.hpp"
#include "../include/controllers/tweet_controller.hpp"
#include "../include/controllers/user_controller.hpp"
#include "../include/middlewares/auth_middleware.hpp"
#include "../include/db.hpp"

int main() {
    crow::App<crow::CORSHandler> app;
    auto& cors = app.get_middleware<crow::CORSHandler>();
    cors.global()
        .origin("http://localhost:5173")
        .methods("GET"_method, "POST"_method, "PUT"_method, "DELETE"_method)
        .headers("Content-Type", "Authorization")
        .allow_credentials();

    Database db("mongodb+srv://prajapatiabhishek13988:FdkPOfDNSblB4pnY@cluster0.rgjlc.mongodb.net/");

    // User Routes
    CROW_ROUTE(app, "/api/v1/user/register").methods("POST"_method)([&db](const crow::request& req) {
        return registerUser(db, req);
    });
    CROW_ROUTE(app, "/api/v1/user/login").methods("POST"_method)([&db](const crow::request& req) {
        return loginUser(db, req);
    });
    CROW_ROUTE(app, "/api/v1/user/logout").methods("GET"_method)([&](const crow::request& req) {
        return logoutUser(req);
    });
    CROW_ROUTE(app, "/api/v1/user/profile/<string>").methods("GET"_method).CROW_MIDDLEWARE(authMiddleware)([&db](const crow::request& req, const std::string& id) {
        crow::request modified_req = req;
        modified_req.url_params = crow::query_string("id=" + id);
        return getProfile(db, modified_req);
    });
    CROW_ROUTE(app, "/api/v1/user/otherusers/<string>").methods("GET"_method).CROW_MIDDLEWARE(authMiddleware)([&db](const crow::request& req, const std::string& id) {
        crow::request modified_req = req;
        modified_req.url_params = crow::query_string("id=" + id);
        return getOthersUsers(db, modified_req);
    });
    CROW_ROUTE(app, "/api/v1/user/bookmark/<string>").methods("PUT"_method).CROW_MIDDLEWARE(authMiddleware)([&db](const crow::request& req, const std::string& id) {
        crow::request modified_req = req;
        modified_req.url_params = crow::query_string("id=" + id);
        return bookmarks(db, modified_req);
    });
    CROW_ROUTE(app, "/api/v1/user/follow/<string>").methods("POST"_method).CROW_MIDDLEWARE(authMiddleware)([&db](const crow::request& req, const std::string& id) {
        crow::request modified_req = req;
        modified_req.url_params = crow::query_string("id=" + id);
        return followUser(db, modified_req);
    });
    CROW_ROUTE(app, "/api/v1/user/unfollow/<string>").methods("POST"_method).CROW_MIDDLEWARE(authMiddleware)([&db](const crow::request& req, const std::string& id) {
        crow::request modified_req = req;
        modified_req.url_params = crow::query_string("id=" + id);
        return unfollowUser(db, modified_req);
    });
    CROW_ROUTE(app, "/api/v1/user/edit/<string>").methods("PUT"_method).CROW_MIDDLEWARE(authMiddleware)([&db](const crow::request& req, const std::string& userId) {
        crow::request modified_req = req;
        modified_req.url_params = crow::query_string("userId=" + userId);
        return editProfile(db, modified_req);
    });
    CROW_ROUTE(app, "/api/v1/user/search").methods("GET"_method).CROW_MIDDLEWARE(authMiddleware)([&db](const crow::request& req) {
        return searchUsers(db, req);
    });
    CROW_ROUTE(app, "/api/v1/user/followrequest/<string>").methods("POST"_method).CROW_MIDDLEWARE(authMiddleware)([&db](const crow::request& req, const std::string& id) {
        crow::request modified_req = req;
        modified_req.url_params = crow::query_string("id=" + id);
        return createFollowRequest(db, modified_req);
    });

    // Tweet Routes
    CROW_ROUTE(app, "/api/v1/tweet/createtweet").methods("POST"_method).CROW_MIDDLEWARE(authMiddleware)([&db](const crow::request& req) {
        return createTweet(db, req);
    });
    CROW_ROUTE(app, "/api/v1/tweet/delete/<string>").methods("DELETE"_method).CROW_MIDDLEWARE(authMiddleware)([&db](const crow::request& req, const std::string& id) {
        crow::request modified_req = req;
        modified_req.url_params = crow::query_string("id=" + id);
        return deleteTweet(db, modified_req);
    });
    CROW_ROUTE(app, "/api/v1/tweet/like/<string>").methods("PUT"_method).CROW_MIDDLEWARE(authMiddleware)([&db](const crow::request& req, const std::string& id) {
        crow::request modified_req = req;
        modified_req.url_params = crow::query_string("id=" + id);
        return likeOrDislike(db, modified_req);
    });
    CROW_ROUTE(app, "/api/v1/tweet/comment/<string>").methods("POST"_method).CROW_MIDDLEWARE(authMiddleware)([&db](const crow::request& req, const std::string& id) {
        crow::request modified_req = req;
        modified_req.url_params = crow::query_string("id=" + id);
        return commentOnTweet(db, modified_req);
    });
    CROW_ROUTE(app, "/api/v1/tweet/alltweets/<string>").methods("GET"_method).CROW_MIDDLEWARE(authMiddleware)([&db](const crow::request& req, const std::string& id) {
        crow::request modified_req = req;
        modified_req.url_params = crow::query_string("id=" + id);
        return getAllTweets(db, modified_req);
    });
    CROW_ROUTE(app, "/api/v1/tweet/followingtweets/<string>").methods("GET"_method).CROW_MIDDLEWARE(authMiddleware)([&db](const crow::request& req, const std::string& id) {
        crow::request modified_req = req;
        modified_req.url_params = crow::query_string("id=" + id);
        return getFollowingTweets(db, modified_req);
    });

    // Notification Routes
    CROW_ROUTE(app, "/api/v1/notification/followrequest/<string>").methods("POST"_method).CROW_MIDDLEWARE(authMiddleware)([&db](const crow::request& req, const std::string& id) {
        crow::request modified_req = req;
        modified_req.url_params = crow::query_string("id=" + id);
        return createFollowRequest(db, modified_req);
    });
    CROW_ROUTE(app, "/api/v1/notification/respond/<string>").methods("POST"_method).CROW_MIDDLEWARE(authMiddleware)([&db](const crow::request& req, const std::string& id) {
        crow::request modified_req = req;
        modified_req.url_params = crow::query_string("id=" + id);
        return respondToFollowRequest(db, modified_req);
    });
    CROW_ROUTE(app, "/api/v1/notification/<string>").methods("GET"_method).CROW_MIDDLEWARE(authMiddleware)([&db](const crow::request& req, const std::string& id) {
        crow::request modified_req = req;
        modified_req.url_params = crow::query_string("id=" + id);
        return getNotifications(db, modified_req);
    });

    // Error Handling
    app.catchall([](const crow::request& req, crow::response& res) {
        res.code = 500;
        res.write(R"({"message": "Internal Server Error", "success": false})");
        res.end();
    });

    app.port(8082).multithreaded().run();
    return 0;
}