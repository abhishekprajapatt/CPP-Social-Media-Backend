#ifndef TWEET_CONTROLLER_HPP
#define TWEET_CONTROLLER_HPP

#include <crow.h>
#include "../../include/db.hpp"

crow::response createTweet(Database& db, const crow::request& req);
crow::response deleteTweet(Database& db, const crow::request& req);
crow::response likeOrDislike(Database& db, const crow::request& req);
crow::response commentOnTweet(Database& db, const crow::request& req);
crow::response getAllTweets(Database& db, const crow::request& req);
crow::response getFollowingTweets(Database& db, const crow::request& req);

#endif