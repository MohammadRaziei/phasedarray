#include "httplib.h"
#include "inja/inja.hpp"
#include <fstream>
#include <iostream>

std::string read_file(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filepath);
    }
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

int main() {
    httplib::Server svr;

    svr.Get("/api/hello", [](const httplib::Request& req, httplib::Response& res) {
        nlohmann::json data;
        data["message"] = "Hello from REST API!";
        res.set_content(data.dump(), "application/json");
    });

    svr.Get("/", [](const httplib::Request& req, httplib::Response& res) {
        try {
            std::string template_str = read_file("src/templates/template.html");

            nlohmann::json data;
            data["name"] = "World";
            data["is_admin"] = true;

            std::string result = inja::render(template_str, data);

            res.set_content(result, "text/html");
        } catch (const std::exception& e) {
            res.set_content("Error: " + std::string(e.what()), "text/plain");
        }
    });

    
    int port = 8080;
    std::cout << "Server running at http://localhost:" << port << std::endl;
    svr.listen("0.0.0.0", port);
    return 0;
}