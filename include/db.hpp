#ifndef DB_HPP
#define DB_HPP

#include <mongocxx/client.hpp>
#include <mongocxx/database.hpp>
#include <string>

class Database {
public:
    Database(const std::string& uri);
    mongocxx::database getDB() const;

private:
    mongocxx::client client;
    mongocxx::database db;
};

#endif