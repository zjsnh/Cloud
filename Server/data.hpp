#pragma once

#include <unordered_map>
#include <pthread.h>

#include "util.hpp"
#include "config.hpp"


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

            return true;
        }


    } BackupInfo;


    class DataManager
    {
    private:
        pthread_rwlock_t rwlock;
        std::unordered_map<std::string, BackupInfo> _table;
        std::string _backup_file;

    public:
        DataManager()
        {
            pthread_rwlock_init(&rwlock, NULL);
            Config *con = Config::GetInstance();
            _backup_file = con->GetBackFile();

            //加载文件内存储的数据
            InitLoad();
        }

        ~DataManager()
        {
            pthread_rwlock_destroy(&rwlock);
        }

        bool Storage()
        {
            Json::Value root;
            // auto it = _table.begin();
            // while (it != _table.end())
            // {
            //     Json::Value item;
            //     item["pack_flag"] = (it->second).pack_flag;
            //     item["fsize"] = (Json::Int64)(it->second).fsize;
            //     item["atime"] = (Json::Int64)(it->second).Atime;
            //     item["mtime"] = (Json::Int64)(it->second).Mtime;
            //     item["real_path"] = (it->second).real_path;
            //     item["pack_path"] = (it->second).pack_path;
            //     item["url_path"] = (it->second).url_path;
            //     root.append(item);
            // }

            std::vector<BackupInfo> array;
            GetAll(&array);

            for (int i = 0; i < array.size();i++)
            {
                Json::Value item;
                item["pack_flag"] = array[i].pack_flag;

                item["fsize"] = (Json::Int64)array[i].fsize;
                item["atime"] = (Json::Int64)array[i].Atime;
                item["mtime"] = (Json::Int64)array[i].Mtime;

                item["real_path"] = array[i].real_path;
                item["pack_path"] = array[i].pack_path;
                item["url_path"] = array[i].url_path;

                root.append(item);
            }

            std::string body;
            if (JsonUtil::Serialize(root, &body) == false)
            {
                std::cout << "Storage Serialize false: " << std::endl;
                return false;
            }

            FileUtil fu(_backup_file);
            fu.SetContent(body);

            return true;
        }

        bool InitLoad()
        {
            std::string body;
            FileUtil fu(_backup_file);
            if(fu.Exists() == false)
            {
                std::cout << "InitLoad file unExists : "<< _backup_file << std::endl;
                return false;
            }

            fu.GetContent(&body);

            Json::Value root;
            if(JsonUtil::UnSerialize(body, &root) == false)
            {
                std::cout << "InitLoad UnSerialize false: " << std::endl;
                return false;
            }

            for (int i = 0; i < root.size();i++)
            {
                BackupInfo info;
                info.pack_flag = root[i]["pack_flag"].asBool();
                info.Atime = root[i]["atime"].asInt64();
                info.fsize = root[i]["fsize"].asInt64();
                info.Mtime = root[i]["mtime"].asInt64();

                info.real_path = root[i]["real_path"].asString();
                info.pack_path = root[i]["pack_path"].asString();
                info.url_path = root[i]["url_path"].asString();

                _table[info.url_path] = info;
            }

            return true;
        }

        bool Insert(const BackupInfo& info)
        {
            pthread_rwlock_wrlock(&rwlock);
            _table[info.url_path] = info;
            pthread_rwlock_unlock(&rwlock);

            Storage();
            return true;
        }

        bool Updata(const BackupInfo& info)
        {
            pthread_rwlock_wrlock(&rwlock);
            _table[info.url_path] = info;
            pthread_rwlock_unlock(&rwlock);

            Storage();
            return true;
        }

        bool GetOneByURL(const std::string& URL,BackupInfo* info)
        {
            pthread_rwlock_wrlock(&rwlock);
            auto it = _table.find(URL);
            if (it == _table.end())
            {
                std::cout << ":: cur" << URL << std::endl;
                pthread_rwlock_unlock(&rwlock);
                std::cout<<"GetOneByURL: URL void"<<std::endl;
                return false;
            }

            *info = it->second;
            pthread_rwlock_unlock(&rwlock);
            return true;
        }

        bool GetOneByRealPath(const std::string& Real_path,BackupInfo* info)
        {
            pthread_rwlock_wrlock(&rwlock);
            auto it = _table.begin();
            while (it != _table.end())
            {
                if(it->second.real_path == Real_path)
                {
                    *info = it->second;
                    pthread_rwlock_unlock(&rwlock);
                    return true;
                }
                it++;
            }
            pthread_rwlock_unlock(&rwlock);
            std::cout<<"GetOneByRealPath: URL void"<<std::endl;
            return false;
        }

        bool GetAll(std::vector<BackupInfo>* arry_info)
        {
            pthread_rwlock_wrlock(&rwlock);
            auto it = _table.begin();
            while (it != _table.end())
            {
                arry_info->push_back(it->second);
                it++;
            }

            pthread_rwlock_unlock(&rwlock);
            return true;
        }
    };
}