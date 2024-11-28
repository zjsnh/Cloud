#pragma once

#include <unordered_map>
#include <pthread.h>
#include <mysql/mysql.h>

#include "util.hpp"
#include "config.hpp"
#include "sql.h"


namespace Cloud
{
    typedef struct BackupInfo_t
    {
        bool pack_flag;
        size_t fsize;
        time_t Atime;
        time_t Mtime;

        std::string real_path;
        std::string pack_path;   //  ./pack_path/ filename .lz
        std::string url_path;    //  ./download/  filename

        std::string filename;

        bool NewBackupInfo(const std::string& realpath)
        {
            //std::cout << realpath << std::endl;

            FileUtil fu(realpath);
            if(fu.Exists() == false)
            {
                std::cout << "NewBackuoInfo file unExists : "<< realpath << std::endl;
                return false;
            }

            pack_flag = false;
            
            fsize = fu.FileSize();
            Atime = fu.LastATime();
            Mtime = fu.LastMTime();
            

            Config *con = Config::GetInstance();
            pack_path = con->GetPackDir() + fu.FileName() + con->GetArcSuffix();
            real_path = realpath;
            url_path = con->GetURLPrefix() + fu.FileName();

            filename = fu.FileName();

            return true;
        }


    } BackupInfo;


    class DataManager
    {
    private:
        pthread_rwlock_t rwlock;
        //std::unordered_map<std::string, BackupInfo> _table;
        //std::string _backup_file;

        MYSQL *mysql = sql::GetInstance();

    public:
        DataManager()
        {
            pthread_rwlock_init(&rwlock, NULL);
            Config *con = Config::GetInstance();
            //_backup_file = con->GetBackFile();

            //加载文件内存储的数据
        }

        ~DataManager()
        {
            pthread_rwlock_destroy(&rwlock);
        }


        bool Mysql_insert(const BackupInfo& info,const std::string& username)
        {
            std::string filename = info.filename;
            std::string file_size = std::to_string(info.fsize);
            std::string last_access_time = std::to_string(info.Atime);
            std::string modified_time = std::to_string(info.Mtime);  // 修正时间格式
            std::string compressed_status = std::to_string(info.pack_flag);  // 假设1表示压缩状态
            std::string real_path = info.real_path;
            std::string pack_path = info.pack_path;
            std::string url_path = info.url_path;
            
            //插入文件，以及文件映射

            std::string insert_query = "INSERT INTO file_info (filename, fsize, Atime, Mtime, pack_flag, real_path, pack_path, url_path) "
                            "VALUES ('" + filename + "', " 
                            + file_size + ", '" 
                            + last_access_time + "', '" 
                            + modified_time + "', " 
                            + compressed_status + ", '" 
                            + real_path + "', '" 
                            + pack_path + "', '" 
                            + url_path + "') "
                            "ON DUPLICATE KEY UPDATE "
                            "fsize = VALUES(fsize), "
                            "Atime = VALUES(Atime), "
                            "Mtime = VALUES(Mtime), "
                            "pack_flag = VALUES(pack_flag), "
                            "real_path = VALUES(real_path), "
                            "pack_path = VALUES(pack_path), "
                            "url_path = VALUES(url_path)";

            if(mysql_query(mysql, insert_query.c_str()))
            {
                std::cerr << "Result storing failed: " << mysql_error(mysql) << "\n";
                return false;
            }

            // std::cout << username << std::endl;
            // std::cout << filename << std::endl;


            insert_query = "INSERT INTO UserFileOwnership (UserName, filename) VALUES ('" + username + "', '" + filename + "');";

            std::cout << insert_query << std::endl;
            if (mysql_query(mysql, insert_query.c_str()))
            {
                std::cerr << "Result storing failed: " << mysql_error(mysql) << "\n";
                return false;
            }

            if (username != "admin")
            {
                insert_query = "INSERT INTO UserFileOwnership (UserName, filename) VALUES ('admin', '" + filename + "');";
                if(mysql_query(mysql, insert_query.c_str()))
                {
                    std::cerr << "Result storing failed: " << mysql_error(mysql) << "\n";
                    return false;
                }
            }

            return true;
        }

        bool Insert(const BackupInfo& info,const std::string& usernanme)
        {
            pthread_rwlock_wrlock(&rwlock);
            Mysql_insert(info,usernanme);
            pthread_rwlock_unlock(&rwlock);

            return true;
        }

        bool Mysql_Updata(const BackupInfo& info)
        {
            std::string flag = std::to_string(info.pack_flag);
            std::string update_query = "UPDATE file_info SET pack_flag = " + flag +
                                       " WHERE filename = '" + info.filename + "'";

            if(mysql_query(mysql, update_query.c_str()))
            {
                std::cerr << "Mysql_Updata :Result storing failed: " << mysql_error(mysql) << "\n";
                return false;
            }

            return true;
        }

        bool Updata(const BackupInfo& info)
        {
            pthread_rwlock_wrlock(&rwlock);
            Mysql_Updata(info);
            pthread_rwlock_unlock(&rwlock);

            return true;
        }

        bool to_BackupInfo(MYSQL_RES* result, BackupInfo* info)
        {
            int num_fields = mysql_num_fields(result);

            if (mysql_num_rows(result) > 0) {
                MYSQL_ROW row = mysql_fetch_row(result); 

                if (row != NULL) {
                    info->filename = (row[0] != NULL) ? row[0] : "";
                    info->pack_flag = (row[1] != NULL) ? atoi(row[1]) : 0; 
                    info->fsize = (row[2] != NULL) ? atoi(row[2]) : 0;
                    info->Atime = (row[3] != NULL) ? atoi(row[3]) : 0;
                    info->Mtime = (row[4] != NULL) ? atoi(row[4]) : 0;
                    info->real_path = (row[5] != NULL) ? row[5] : "";
                    info->pack_path = (row[6] != NULL) ? row[6] : "";
                    info->url_path = (row[7] != NULL) ? row[7] : "";
                    
                    mysql_free_result(result);
                    return true;
                } else {
                    std::cout << "Error: No valid row data found." << "\n";
                    mysql_free_result(result);
                    return false;
                }
            } else {
                std::cout << "No rows found in the result." << "\n";
                mysql_free_result(result);
                return false;
            }
        }


        bool select(const std::string& tables ,const std::string& sql,const std::string& sql_from,BackupInfo* info)
        {
            MYSQL_RES *result;
            sql::Mysql_select(tables, sql, sql_from,result);
            // 把结果转化为 BackupInfo
            to_BackupInfo(result, info);
            return true;
        }

        bool GetOneByURL(const std::string& URL,BackupInfo* info)
        {
            pthread_rwlock_wrlock(&rwlock);

            if(select("file_info",URL, "url_path", info) ==false)
            {
                pthread_rwlock_unlock(&rwlock);
                std::cout<<"GetOneByURL: URL void"<<std::endl;
                return false;
            }

            pthread_rwlock_unlock(&rwlock);
            return true;
        }

        bool GetOneByRealPath(const std::string& Real_path,BackupInfo* info)
        {
            pthread_rwlock_wrlock(&rwlock);
            
            if(select("file_info",Real_path, "real_path", info) ==false)
            {
                pthread_rwlock_unlock(&rwlock);
                std::cout<<"GetOneByRealPath: real_path void"<<std::endl;
                return false;
            }

            pthread_rwlock_unlock(&rwlock);
            return true;
        }

        // bool Select(MYSQL* mysql,std::vector<BackupInfo>* arry_info,const std::string username)
        // {
        //     //查询对应登录用户的数据，进行格式输出
        //     std::string select_query = "select *from file_info";
        //     mysql_query(mysql, select_query.c_str());
        //     MYSQL_RES *res = mysql_store_result(mysql);
        //     if(res)
        //     {
        //         MYSQL_ROW row;
        //         while((row = mysql_fetch_row(res)))
        //         {
        //             BackupInfo info;
        //             info.filename = row[0];
        //             info.pack_flag = atoi(row[1]);
        //             info.fsize = atoi(row[2]);
        //             info.Atime = atoi(row[3]);
        //             info.Mtime = atoi(row[4]);
        //             info.real_path = row[5];
        //             info.pack_path = row[6];
        //             info.url_path = row[7];
        //             arry_info->push_back(info);
        //         }
        //         mysql_free_result(res);
        //     }
        //     else{
        //         std::cerr << "Result storing failed: " << mysql_error(mysql) << "\n";
        //     }
        //     return true;
        // }

        bool Select(MYSQL* mysql, std::vector<BackupInfo>* arry_info, const std::string username)
        {
            // 查询用户拥有的文件，使用JOIN将三个表连接起来
            std::string select_query = 
                "SELECT f.filename, f.pack_flag, f.fsize, f.Atime, f.Mtime, f.real_path, f.pack_path, f.url_path "
                "FROM Users u "
                "JOIN UserFileOwnership ufo ON u.UserName = ufo.UserName "
                "JOIN file_info f ON ufo.filename = f.filename "
                "WHERE u.UserName = '" + username + "'";

            // 执行查询
            if (mysql_query(mysql, select_query.c_str())) {
                std::cerr << "SELECT query failed. Error: " << mysql_error(mysql) << std::endl;
                return false;
            }

            // 获取查询结果
            MYSQL_RES* res = mysql_store_result(mysql);
            if (res) {
                MYSQL_ROW row;
                while ((row = mysql_fetch_row(res))) {
                    BackupInfo info;
                    info.filename = row[0];
                    info.pack_flag = atoi(row[1]);
                    info.fsize = atoi(row[2]);
                    info.Atime = atoi(row[3]);
                    info.Mtime = atoi(row[4]);
                    info.real_path = row[5];
                    info.pack_path = row[6];
                    info.url_path = row[7];
                    arry_info->push_back(info);
                }
                mysql_free_result(res);
            } else {
                std::cerr << "Result storing failed: " << mysql_error(mysql) << "\n";
                return false;
            }

            return true;
        }

        bool GetAll(std::vector<BackupInfo>* arry_info,const std::string& username)
        {
            pthread_rwlock_wrlock(&rwlock);
            
            if(Select(mysql,arry_info,username) == false)
            {
                pthread_rwlock_unlock(&rwlock);
                std::cout<<"GetOneByRealPath: real_path void"<<std::endl;
                return false;
            }

            pthread_rwlock_unlock(&rwlock);
            return true;
        }
    };
}