#include "crow_all.h"
#include "database.hpp"
#include "logger.hpp"
#include "utils.hpp"
#include "config.hpp"
#include <string>

void setup_routes(crow::SimpleApp& app, int short_code_length) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 61);
    const std::string chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    auto generate_short_lambda = [&]() -> std::string {
        std::string short_code;
        for (int i = 0; i < short_code_length; ++i) {
            short_code += chars[dis(gen)];
        }
        while (!get_url(short_code).empty()) {
            short_code = "";
            for (int i = 0; i < short_code_length; ++i) {
                short_code += chars[dis(gen)];
            }
        }
        return short_code;
    };

    CROW_ROUTE(app, "/shorten")
        .methods("POST"_method)
        ([&](const crow::request& req) {
            std::string ip = req.get_header_value("X-Forwarded-For");
            if (ip.empty()) ip = req.get_header_value("X-Real-IP");
            if (ip.empty()) ip = "unknown";
            std::string ua = req.get_header_value("User-Agent");
            log("Received shorten request", "INFO", ip, ua);
            auto body = crow::json::load(req.body);
            if (!body || !body.has("url")) {
                log("Invalid request: missing or invalid JSON", "WARN", ip, ua);
                return crow::response(400, "Invalid JSON or missing 'url' field");
            }
            std::string url = body["url"].s();
            if (url.empty() || url.find("http") != 0) {
                log("Invalid URL: " + url, "WARN", ip, ua);
                return crow::response(400, "Invalid URL");
            }
            std::string existing_code = get_short_code(url);
            if (!existing_code.empty()) {
                log("URL already shortened: " + url + " -> " + existing_code, "INFO", ip, ua);
                crow::json::wvalue response;
                response["short_url"] = "http://localhost:8080/" + existing_code;
                return crow::response(response);
            }
            std::string short_code = generate_short_lambda();
            insert_url(short_code, url);
            log("Shortened URL: " + url + " to " + short_code, "INFO", ip, ua);
            crow::json::wvalue response;
            response["short_url"] = "http://localhost:8080/" + short_code;
            return crow::response(response);
        });

    CROW_ROUTE(app, "/<string>")
        .methods("GET"_method)
        ([&](const crow::request& req, std::string short_code) {
            std::string ip = req.get_header_value("X-Forwarded-For");
            if (ip.empty()) ip = req.get_header_value("X-Real-IP");
            if (ip.empty()) ip = "unknown";
            std::string ua = req.get_header_value("User-Agent");
            if (short_code.empty()) {
                log("Invalid short code request", "WARN", ip, ua);
                return crow::response(400, "Invalid short code");
            }
            log("Redirect request for: " + short_code, "INFO", ip, ua);
            std::string url = get_url(short_code);
            if (!url.empty()) {
                log("Redirecting to: " + url, "INFO", ip, ua);
                crow::response res(302);
                res.add_header("Location", url);
                return res;
            } else {
                log("Short URL not found: " + short_code, "WARN", ip, ua);
                return crow::response(404, "Short URL not found");
            }
        });

    CROW_ROUTE(app, "/delete/<string>")
        .methods("DELETE"_method)
        ([&](const crow::request& req, std::string short_code) {
            std::string ip = req.get_header_value("X-Forwarded-For");
            if (ip.empty()) ip = req.get_header_value("X-Real-IP");
            if (ip.empty()) ip = "unknown";
            std::string ua = req.get_header_value("User-Agent");
            if (short_code.empty()) {
                log("Invalid delete request", "WARN", ip, ua);
                return crow::response(400, "Invalid short code");
            }
            log("Delete request for: " + short_code, "INFO", ip, ua);
            std::string url = get_url(short_code);
            if (!url.empty()) {
                delete_url(short_code);
                log("Deleted URL: " + short_code, "INFO", ip, ua);
                return crow::response(200, "Deleted");
            } else {
                log("Short URL not found: " + short_code, "WARN", ip, ua);
                return crow::response(404, "Short URL not found");
            }
        });
}