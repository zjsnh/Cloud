#pragma once

#include "data.hpp"
#include <unistd.h>


extern Cloud::DataManager *data;

namespace Cloud
{

    class HotManager{
    private:
        std::string _back_dir;
        std::string _pack_dir;
        std::string _pack_suffix;

        int _hot_time;

        bool HotJudge(const std::string& filename)
        {
            FileUtil fu(filename);
            time_t last_atime = fu.LastATime();  //访问
            time_t loacl_time = time(NULL);

            if(loacl_time - last_atime > _hot_time)
            {
                // 非热点文章返回true
                return true;
            }

            return false;
        }

    public:
        HotManager()
        {
            Config *con = Config::GetInstance();
            _back_dir = con->GetBackDir();
            _pack_dir = con->GetPackDir();
            _pack_suffix = con->GetArcSuffix();

            _hot_time = con->GetHotTime();
            //std::cout << _hot_time << std::endl;
        }

        bool RunModule()
        {
            while(true)
            {
                FileUtil fu(_back_dir);
                
                std::vector<std::string> array;
                fu.DirectoryFile(array);

                for (auto &a : array)
                {
                    if(!HotJudge(a))
                    {
                        continue;
                    }

                    BackupInfo bi;
                    if (data->GetOneByRealPath(a, &bi) == false)
                    {
                        bi.NewBackupInfo(a);
                    }

                    FileUtil temp(a);
                    temp.compare(bi.pack_path);
                    temp.Remove();

                    bi.pack_flag = true;
                    data->Updata(bi);
                }

                sleep(3);
            }

            return true;
        }
    };
}