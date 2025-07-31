#ifndef AUTH_MIDDLEWARE_HPP
#define AUTH_MIDDLEWARE_HPP

#include <crow.h>
#include <jwt-cpp/jwt.h>

crow::middleware::function authMiddleware();

#endif