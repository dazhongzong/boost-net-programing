#include <iostream>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
int main()
{
    Json::Value root;
    root["id"] = 1001;
    root["value"] = "hello world";
    std::string request = root.toStyledString();
    std::cout<<"request is :\n"<<request<<std::endl;
    Json::Value root2;
    Json::Reader reader;
    reader.parse(request,root2);
    std::cout<<"msg id is "<<root2["id"] <<" value is "<< root2["value"]<<std::endl;
    
    return 0;
}