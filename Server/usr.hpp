#pragma once


#include "util.hpp"
#include "sql.h"
#include "httplib.h"

namespace Cloud
{
    class cookie
    {
    public:
        static bool verify(std::string username,std::string password)
        {
            //std::cout << password << std::endl;
            MYSQL *mysql = sql::GetInstance();
            std::string query = "select * from Users where UserName = '" + username + "'";
            if (mysql_query(mysql, query.c_str())) {
                std::cerr << "Query failed: " << mysql_error(mysql) << "\n";
                return false;
            }

            MYSQL_RES* result = mysql_store_result(mysql);
            if (!result) {
                std::cerr << "Result storing failed: " << mysql_error(mysql) << "\n";
                return false;
            }

            if(mysql_num_rows(result) > 0)
            {
                MYSQL_ROW row = mysql_fetch_row(result);
                if (row != NULL && row[1] == password)
                {
                    std::cout << "登录完成" << std::endl;
                    return true;
                }
            }

            return false;
        }

        
        static std::string get_cookie(const httplib::Request& req,const std::string& get) 
        {
            std::unordered_map<std::string, std::string> cookies;
            std::string cookie = req.get_header_value("Cookie");

            std::cout << cookie << std::endl;

            size_t pos = 0, idx = 0;
            std::string sep = ";";
            while (true)
            {
                pos = cookie.find(";", idx);
                if (pos == std::string::npos)
                    break;

                std::string _str = cookie.substr(idx, pos);
                size_t pos_ = _str.find("=");
                cookies[_str.substr(0, pos_)] = _str.substr(pos_ + 1);

                cookie = cookie.substr(pos + 2);
            }
            if (cookie.empty() == false)
            {
                size_t pos_ = cookie.find("=");
                cookies[cookie.substr(0, pos_)] = cookie.substr(pos_ + 1);
            }

            return cookies[get];
        }

        static bool cookie_verify(const httplib::Request& req,httplib::Response& res)
        {
            std::string cookie = req.get_header_value("Cookie");
            if (cookie.empty() && req.path != "/login.html")
            {
                std::string cookie_path = "cookie_path=" + req.path;
                res.set_header("Set-Cookie", cookie_path);
                res.set_header("Location", "http://123.60.163.45:8888/login.html");
                res.status = 302;  // 设置 HTTP 状态码为 302，表示重定向
                //std::cout << "没有 Cookie,重定向到登录页面...\n";
                return true;
            }

            return false;
        }
    };
    


}