Social Media Backend (C++)

A high-performance C++ backend for a social media application, replicating a Node.js implementation. Supports user authentication, tweeting, following, notifications, and image uploads with modern C++ libraries.
Table of Contents

Features
Tech Stack
Prerequisites
Installation
Configuration
Running the Backend
API Endpoints
Contributing
License

Features

User registration, login, and profile management.
Tweet creation, deletion, liking, and commenting.
Follow/unfollow users with private account requests.
Bookmark tweets and search users.
Image uploads with resizing via Cloudinary.
JWT-based authentication and MongoDB storage.

Tech Stack

C++17: Core language.
Crow: Web framework.
mongocxx: MongoDB driver.
nlohmann/json: JSON handling.
jwt-cpp: JWT authentication.
libvips: Image processing.
cpp-httplib: HTTP client for Cloudinary.
libcrypto++: Password hashing (bcrypt).
CMake: Build system.
vcpkg: Dependency management.

Prerequisites

Windows 10/11 (adjustments needed for other platforms).
MongoDB (local or Atlas).
Cloudinary account for image uploads.
Git, CMake, Visual Studio 2022 (C++ workload).
~25 GB free disk space.

Installation

Clone the repository:
git clone https://github.com/your-username/social-media-backend.git
cd social-media-backend


Install tools:

Visual Studio 2022 with "Desktop development with C++".
CMake from cmake.org (add to PATH).
vcpkg:git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
cd C:\vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install




Install dependencies:
cd C:\vcpkg
.\vcpkg install mongo-cxx-driver:x64-windows libvips:x64-windows curl:x64-windows cryptopp:x64-windows


Add header-only libraries:
Crow: crow_all.h in include/crow/.
nlohmann/json: json.hpp in include/nlohmann/.
jwt-cpp: Headers in include/jwt-cpp/.
cpp-httplib: httplib.h in include/httplib/.




Set up MongoDB:

Install MongoDB Community or use Atlas.
Create collections:use social_media
db.createCollection("users")
db.createCollection("tweets")
db.createCollection("notifications")
db.users.createIndex({ email: 1 }, { unique: true })
db.users.createIndex({ username: 1 }, { unique: true })





Configuration

Edit config/config.env with your credentials:MONGO_URI=mongodb+srv://<user>:<password>@cluster0.rgjlc.mongodb.net/
SECRET_KEY=your_jwt_secret
CLOUD_NAME=your_cloud_name
API_KEY=your_api_key
API_SECRET=your_api_secret
PORT=8082
FRONTEND_URL=http://localhost:5173


Update src/utils/cloudinary.cpp with your Cloudinary upload_preset.
Set VITE_BACKEND_URL=http://localhost:8082 in your frontend (e.g., React/Vite).

Running the Backend

Build the project:
cd social-media-backend
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake ..
cmake --build . --config Release


Start the server:
cd Release
backend.exe


Runs on http://localhost:8082.


Test endpoints:
curl -X POST http://localhost:8082/api/v1/user/register -H "Content-Type: application/json" -d '{"name":"Test User","username":"testuser","email":"test@example.com","password":"password123"}'



API Endpoints



Endpoint
Method
Description
Authentication



/api/v1/user/register
POST
Register user
None


/api/v1/user/login
POST
Login with JWT
None


/api/v1/user/logout
GET
Logout
None


/api/v1/user/profile/<id>
GET
Get profile
JWT


/api/v1/user/edit/<id>
PUT
Update profile
JWT


/api/v1/user/follow/<id>
POST
Follow user
JWT


/api/v1/user/unfollow/<id>
POST
Unfollow user
JWT


/api/v1/user/bookmark/<id>
PUT
Bookmark tweet
JWT


/api/v1/user/search?query=<term>
GET
Search users
JWT


/api/v1/tweet/createtweet
POST
Create tweet
JWT


/api/v1/tweet/delete/<id>
DELETE
Delete tweet
JWT


/api/v1/tweet/like/<id>
PUT
Like/unlike tweet
JWT


/api/v1/tweet/comment/<id>
POST
Comment on tweet
JWT


/api/v1/tweet/alltweets/<id>
GET
Get user tweets
JWT


/api/v1/tweet/followingtweets/<id>
GET
Get following tweets
JWT


/api/v1/notification/followrequest/<id>
POST
Send follow request
JWT


/api/v1/notification/respond/<id>
POST
Respond to request
JWT


/api/v1/notification/<id>
GET
Get notifications
JWT


Contributing
Contributions welcome! Steps:

Fork the repository.
Create a branch: git checkout -b feature/YourFeature.
Commit changes: git commit -m "Add YourFeature".
Push: git push origin feature/YourFeature.
Open a pull request.

License
MIT License