cmake_minimum_required(VERSION 3.10)
project(SocialMediaBackend)

set(CMAKE_CXX_STANDARD 17)

find_package(mongocxx REQUIRED)
find_package(bsoncxx REQUIRED)
find_package(CURL REQUIRED)
find_package(VIPS REQUIRED)
find_package(cryptopp REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${MONGO_CXX_INCLUDE_DIRS})
include_directories(${CURL_INCLUDE_DIRS})
include_directories(${VIPS_INCLUDE_DIRS})
include_directories(${CRYPTOPP_INCLUDE_DIRS})

add_executable(backend
    src/main.cpp
    src/db.cpp
    src/middlewares/auth_middleware.cpp
    src/controllers/notification_controller.cpp
    src/controllers/tweet_controller.cpp
    src/controllers/user_controller.cpp
    src/utils/cloudinary.cpp
    src/utils/image_processor.cpp
)

target_link_libraries(backend
    mongo::mongocxx_shared
    mongo::bsoncxx_shared
    ${CURL_LIBRARIES}
    ${VIPS_LIBRARIES}
    cryptopp::cryptopp
)