#ifndef USER_CONTROLLER_HPP
#define USER_CONTROLLER_HPP

#include <crow.h>
#include "../../include/db.hpp"

crow::response registerUser(Database& db, const crow::request& req);
crow::response loginUser(Database& db, const crow::request& req);
crow::response logoutUser(const crow::request& req);
crow::response bookmarks(Database& db, const crow::request& req);
crow::response editProfile(Database& db, const crow::request& req);
crow::response getProfile(Database& db, const crow::request& req);
crow::response getOthersUsers(Database& db, const crow::request& req);
crow::response followUser(Database& db, const crow::request& req);
crow::response unfollowUser(Database& db, const crow::request& req);
crow::response searchUsers(Database& db, const crow::request& req);

#endif