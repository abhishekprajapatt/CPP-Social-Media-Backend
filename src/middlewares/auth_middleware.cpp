#include "../../include/middlewares/auth_middleware.hpp"
#include <string>

crow::middleware::function authMiddleware() {
    return [](crow::request& req, crow::response& res, std::function<void()> next) {
        auto auth_header = req.get_header_value("Authorization");
        if (auth_header.empty() || auth_header.find("Bearer ") != 0) {
            res.code = 401;
            res.write(R"({"message": "User is not Authenticated!", "success": false})");
            res.end();
            return;
        }

        std::string token = auth_header.substr(7);
        try {
            auto decoded = jwt::decode(token);
            auto verifier = jwt::verify()
                .allow_algorithm(jwt::algorithm::hs256{"tqeetkkdltweetkkd94kkfj43kkfkllwel"})
                .with_issuer("your_app");
            verifier.verify(decoded);
            req.add_header("userId", decoded.get_payload_claim("userId").as_string());
            next();
        } catch (const std::exception& e) {
            res.code = 401;
            res.write(R"({"message": "Invalid token", "success": false})");
            res.end();
        }
    };
}