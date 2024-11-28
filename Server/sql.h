#pragma once

#include <mysql/mysql.h>
#include <string>
#include <iostream>
#include <mutex>

namespace Cloud
{
    class sql
    {
    private:
        static std::mutex _mutex;
        static MYSQL *mysql;

        sql();
        ~sql();

    public:

        static MYSQL* GetInstance()
        {
            if(mysql == NULL)
            {
                _mutex.lock();
                if(mysql == NULL)
                {
                    mysql = mysql_init(NULL);
                    if(mysql == NULL)
                    {
                        std::cout << "mysql initialization failed \n";
                        return NULL;
                    }

                    mysql = mysql_real_connect(mysql, "localhost", "zws", "123456", "Cloud", 3306, NULL, 0);
                    if (mysql == NULL)
                    {
                        std::cout << "mysql connect failed \n";
                        return NULL;
                    }

                    return mysql;
                }

                _mutex.unlock();
            }

            return mysql;
        }

        static bool Mysql_select(const std::string& tables ,const std::string& sql,const std::string& sql_from,MYSQL_RES*& result)
        {
            std::string query = "select * from " + tables + " where " + sql_from + " = '" + sql + "'";

            if (mysql_query(mysql, query.c_str())) {
                std::cerr << "Query failed: " << mysql_error(mysql) << "\n";
                return false;
            }

            result = mysql_store_result(mysql);
            if (!result) {
                std::cerr << "Result storing failed: " << mysql_error(mysql) << "\n";
                return false;
            }

            return true;
        }
    };

    //静态成员类外定义
    MYSQL* Cloud::sql::mysql = NULL;
    std::mutex Cloud::sql::_mutex;
}