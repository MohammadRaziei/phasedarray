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
            data["url_chart"] = "/chart";
            std::string result = inja::render(template_str, data);

            res.set_content(result, "text/html");
        } catch (const std::exception& e) {
            res.set_content("Error: " + std::string(e.what()), "text/plain");
        }
    });

    svr.Get("/chart", [](const httplib::Request& req, httplib::Response& res) {
        try {
            std::string template_str = read_file("src/templates/highchart_template.html");

            nlohmann::json data;
            data["chart_title"] = "Monthly Sales Chart"; // مقدار از قبل موجود است
            data["chart_type"] = "column";             // مقدار از قبل موجود است
            
            nlohmann::json categories_json_object = nlohmann::json::array();
            categories_json_object.push_back("January");
            categories_json_object.push_back("February");
            categories_json_object.push_back("March");
            categories_json_object.push_back("April");
            categories_json_object.push_back("May");
            data["categories_as_string"] = categories_json_object.dump(); // تبدیل به رشته JSON

            nlohmann::json series_data_json_object = nlohmann::json::array();
            
            nlohmann::json series1;
            series1["name"] = "Product A";
            nlohmann::json series1_data = nlohmann::json::array({10, 15, 7, 12, 20});
            series1["data"] = series1_data;
            series_data_json_object.push_back(series1);

            nlohmann::json series2;
            series2["name"] = "Product B";
            nlohmann::json series2_data = nlohmann::json::array({5, 8, 13, 10, 16});
            series2["data"] = series2_data;
            series_data_json_object.push_back(series2);
            
            data["series_data_as_string"] = series_data_json_object.dump(); // تبدیل به رشته JSON

            std::string result = inja::render(template_str, data);

            res.set_content(result, "text/html; charset=utf-8");
        } catch (const std::exception& e) {
            res.set_content("Error: " + std::string(e.what()), "text/plain; charset=utf-8");
        }
    });

    
    int port = 8080;
    std::cout << "Server running at http://localhost:" << port << std::endl;
    svr.listen("0.0.0.0", port);
    return 0;
}