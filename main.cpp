#include "crow_all.h"
#include <unordered_map>
#include <string>
#include <atomic>

int main() {
    crow::SimpleApp app;
    std::unordered_map<std::string, std::string> url_map;
    std::atomic<size_t> counter{0};

    auto generate_short = [&]() -> std::string {
        size_t id = counter++;
        std::string short_code;
        const std::string chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        if (id == 0) return "0";
        while (id > 0) {
            short_code = chars[id % chars.size()] + short_code;
            id /= chars.size();
        }
        return short_code;
    };

    CROW_ROUTE(app, "/shorten")
        .methods("POST"_method)
        ([&](const crow::request& req) {
            auto body = crow::json::load(req.body);
            if (!body || !body.has("url")) {
                return crow::response(400, "Invalid JSON or missing 'url' field");
            }
            std::string url = body["url"].s();
            std::string short_code = generate_short();
            url_map[short_code] = url;
            crow::json::wvalue response;
            response["short_url"] = "http://localhost:8080/" + short_code;
            return crow::response(response);
        });

    CROW_ROUTE(app, "/<string>")
        .methods("GET"_method)
        ([&](std::string short_code) {
            if (url_map.find(short_code) != url_map.end()) {
                crow::response res(302);
                res.add_header("Location", url_map[short_code]);
                return res;
            } else {
                return crow::response(404, "Short URL not found");
            }
        });

    app.port(8080).multithreaded().run();
    return 0;
}