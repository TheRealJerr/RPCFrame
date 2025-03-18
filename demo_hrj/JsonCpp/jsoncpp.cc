// 这个头文件定义了json的序列化和反序列化
#include <iostream>
#include <string>
#include <sstream>
#include <jsoncpp/json/json.h>
#include <memory>
 

// 序列化
bool serialize(const Json::Value& val,std::string& body)
{
    Json::StreamWriterBuilder factory;
    std::unique_ptr<Json::StreamWriter> writer(factory.newStreamWriter());
    std::stringstream ssm;
    int n = writer->write(val,&ssm);

    if(n != 0)
    {
        std::cout << "serialize error..." << std::endl;
        return false;
    }

    body = std::move(ssm.str());// 直接将右值给他
    return true;
}
// 反序列化
bool deserialize(Json::Value& val,const std::string& body)
{
    Json::CharReaderBuilder factory;
    std::unique_ptr<Json::CharReader> reader(factory.newCharReader());
    std::string errors;
    bool ret = reader->parse(body.c_str(),body.c_str() + body.size(),&val,&errors);
    if(ret == false)
    {
        std::cout << "deserialize fail:" << errors << std::endl;
        return false;
    } 
    else
    {
        return true;
    }
}


