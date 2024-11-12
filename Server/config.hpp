#pragma once
#include <mutex>
#include "util.hpp"


namespace Cloud
{
    #define CONFIG "Cloud.conf"
    class Config
    {
    private:
        static std::mutex _mutex;
        static Config *_instance;
        Config()
        {
            ReadConfig();
        }

    private:
        int _hot_time;
        int _server_port;
        std::string _server_ip;
        std::string _url_prefix;
        std::string _arc_suffix;
        std::string _pack_dir;
        std::string _back_dir;
        std::string _backup_file;

        bool ReadConfig()
        {
            FileUtil fu(CONFIG);
            std::string body;
            fu.GetContent(&body);

            Json::Value root;
            JsonUtil::UnSerialize(body, &root);

            _hot_time = root["hot_time"].asInt();
            _server_port = root["server_port"].asInt();
            _server_ip = root["server_ip"].asString();

            _url_prefix = root["url_prefix"].asString();
            _arc_suffix = root["arc_suffix"].asString();

            _pack_dir = root["pack_dir"].asString();
            _back_dir = root["back_dir"].asString();

            _backup_file = root["backup_file"].asString();
        }

    public:
        static Config* GetInstance(){
            if (_instance == NULL)
            {
                _mutex.lock();
                if (_instance == NULL)
                {
                    _instance = new Config();
                }
                _mutex.unlock();
            }

            return _instance;
        }

        int GetHotTime()
        {
            return _hot_time;
        }


        int GetServerPort()
        {
            return _server_port;
        }

        std::string GetServerIp()
        {
            return _server_ip;
        }

        std::string GetURLPrefix()
        {
            return _url_prefix;
        }

        std::string GetArcSuffix()
        {
            return _arc_suffix;
        }

        std::string GetPackDir()
        {
            return _pack_dir;
        }

        std::string GetBackDir()
        {
            return _back_dir;
        }

        std::string GetBackFile()
        {
            return _backup_file;
        }
    };

    //静态成员类外定义
    Cloud::Config* Cloud::Config::_instance = NULL;
    std::mutex Cloud::Config::_mutex;
}