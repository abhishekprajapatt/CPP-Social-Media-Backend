#include "../include/db.hpp"
#include <mongocxx/instance.hpp>
#include <stdexcept>

Database::Database(const std::string& uri) : client(mongocxx::uri(uri)) {
    try {
        db = client["social_media"];
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to connect to MongoDB: " + std::string(e.what()));
    }
}

mongocxx::database Database::getDB() const {
    return db;
}