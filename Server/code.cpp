#include "config.hpp"
#include "data.hpp"
#include "hot.hpp"

#include "server.hpp"
#include <thread>

Cloud::DataManager *data;

void ServiceTest()
{
    Cloud::Server svr;
    svr.RunModule();
}

void HotTest()
{
    Cloud::HotManager *hot = new Cloud::HotManager();
    hot->RunModule();
    return ;
}



int main()
{
    data = new Cloud::DataManager();
    std::thread thread_hot_manager(HotTest);
	std::thread thread_service(ServiceTest);

    thread_hot_manager.join();
    thread_service.join();

    return 0;
}

// int main()
// {
//     data = new Cloud::DataManager();
//     Cloud::HotManager *hot = new Cloud::HotManager();
//     hot->RunModule();
//     return 0;
// }

// int main()
// {
//     // data = new Cloud::DataManager();
//     // Cloud::Server svr;
//     // svr.RunModule();

//     // Cloud::FileUtil fu("./backdir/1.html");
//     // std::string body;
//     // fu.GetContent(&body);

//     // std::cout << body << std::endl;
//     return 0;
// }

// void data_hpp()
// {
//     // Cloud::BackupInfo info;
//     // info.NewBackupInfo("bundle.h");

//     Cloud::DataManager data;
//     // data.Insert(info);
//     // info.NewBackupInfo("util.hpp");
//     // data.Insert(info);

//     std::vector<Cloud::BackupInfo> array;
//     data.GetAll(&array);

//     for(const auto& e:array)
//     {
//         std::cout << e.url_path << std::endl;
//     }

//     return;
// }

// int main()
// {
//     //data_hpp();

//     return 0;
// }