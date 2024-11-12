#include <iostream>
#include "util.hpp"


int main()
{
    // Cloud::FileUtil fu("code");
    // fu.CreateDirectory();

    // std::vector<std::string> files;
    // fu.DirectoryFile(files);

    // for(auto& e:files)
    // {
    //     std::cout << e << std::endl;
    // }

    Json::Value root;
    root["nihao"] = "woshinide";
    root["n"] = 19;
    root["ni"] = "woshinide";
    root["nih"] = "woshinide";

    std::string str;
    Cloud::JsonUtil::Serialize(root, &str);
    std::cout << str << std::endl;

    Json::Value root_;
    Cloud::JsonUtil::UnSerialize(str, &root_);
    std::cout << root_["nihao"].asString() << std::endl;

    return 0;
}