#ifndef NOTIFICATION_CONTROLLER_HPP
#define NOTIFICATION_CONTROLLER_HPP

#include <crow.h>
#include "../../include/db.hpp"

crow::response createFollowRequest(Database& db, const crow::request& req);
crow::response respondToFollowRequest(Database& db, const crow::request& req);
crow::response getNotifications(Database& db, const crow::request& req);

#endif