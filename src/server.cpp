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

// تابع ساخت داده‌های نمودار
nlohmann::json createChartData() {
    nlohmann::json chart_data;
    chart_data["chart_title"] = "Annual Sales Chart";
    chart_data["chart_type"] = "column";
    chart_data["y_axis_title"] = "Sales (Million)";
    
    // ساخت داده‌های دسته‌بندی
    chart_data["categories"] = nlohmann::json::array({"January", "February", "March", "April", "May"});
    
    // ساخت داده‌های سری
    nlohmann::json series = nlohmann::json::array();
    
    nlohmann::json series1;
    series1["name"] = "Product A";
    series1["data"] = nlohmann::json::array({10, 15, 7, 12, 20});
    series.push_back(series1);

    nlohmann::json series2;
    series2["name"] = "Product B";
    series2["data"] = nlohmann::json::array({5, 8, 13, 10, 16});
    series.push_back(series2);
    
    chart_data["series"] = series;
    
    return chart_data;
}

int main() {
    httplib::Server svr;
    inja::Environment env("src/templates/");
    env.add_callback("json.dumps", 1, [](inja::Arguments& args) {
        nlohmann::json data = args.at(0)->get<nlohmann::json>(); // Adapt the index and type of the argument
        return data.dump(2);
    });


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

    svr.Get("/content", [&env](const httplib::Request& req, httplib::Response& res) {
        try {
            // Corrected code:
            nlohmann::json data;   // Prepare data for the template (if any)
            
            std::string result = env.render_file("content.html", data);

            res.set_content(result, "text/html");
        } catch (const std::exception& e) {
            res.set_content("Error: " + std::string(e.what()), "text/plain");
        }
    });


    // API endpoint for chart data
    svr.Get("/chart_data", [](const httplib::Request& req, httplib::Response& res) {
        try {
            nlohmann::json data = createChartData();
            res.set_content(data.dump(), "application/json; charset=utf-8");
        } catch (const std::exception& e) {
            nlohmann::json error;
            error["error"] = e.what();
            res.set_content(error.dump(), "application/json; charset=utf-8");
            res.status = 500;
        }
    });

    // Route for displaying chart
    svr.Get("/chart", [&env](const httplib::Request& req, httplib::Response& res) {
        try {
            nlohmann::json data;
            data["page_title"] = "Sales Chart";
            data["header_title"] = "Sales Chart";
            data["footer_text"] = "Sales Chart";
            
            data["data"] = createChartData();
            data["chart_title"] = "Annual Sales Chart";
            
            std::string result = env.render_file("chart_page.html", data);
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
