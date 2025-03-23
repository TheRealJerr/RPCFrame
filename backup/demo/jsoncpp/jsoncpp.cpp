#include <iostream>
#include <sstream>
#include <string>
#include <memory>
#include <jsoncpp/json/json.h>

//实现数据的序列化
bool serialize(const Json::Value &val,  std::string &body) {
    std::stringstream ss;
    //先实例化一个工厂类对象
    Json::StreamWriterBuilder swb;
    //通过工厂类对象来生产派生类对象
    std::unique_ptr<Json::StreamWriter> sw(swb.newStreamWriter());
    int ret = sw->write(val, &ss);
    if (ret != 0) {
        std::cout << "json serialize failed!\n";
        return false;
    }
    body = ss.str();
    return true;
}

//实现json字符串的反序列化
bool unserialize(const std::string &body, Json::Value &val) {
    //实例化工厂类对象
    Json::CharReaderBuilder crb;
    //生产CharReader对象
    std::string errs;
    std::unique_ptr<Json::CharReader> cr(crb.newCharReader());
    bool ret = cr->parse(body.c_str(), body.c_str() + body.size(), &val, &errs);
    if (ret == false) {
        std::cout << "json unserialize failed : " << errs << std::endl;
        return false;
    }
    return true;
}

int main()
{
    const char *name = "小明";
    int age = 18; 
    const char *sex = "男";
    float score[3] = {88, 77.5, 66};

    Json::Value student;
    student["姓名"] = name;
    student["年龄"] = age;
    student["性别"] = sex;
    student["成绩"].append(score[0]);
    student["成绩"].append(score[1]);
    student["成绩"].append(score[2]);
    Json::Value fav;
    fav["书籍"] = "西游记";
    fav["运动"] = "打篮球";
    student["爱好"] = fav;
    std::string body;

    serialize(student, body);
    std::cout << body << std::endl;

    std::string str = R"({"姓名":"小黑", "年龄": 19, "成绩":[32, 45, 56]})";
    Json::Value stu;
    bool ret = unserialize(str, stu);
    if (ret == false) 
        return -1;
    std::cout << "姓名: " <<  stu["姓名"].asString() << std::endl;
    std::cout << "年龄: " <<  stu["年龄"].asInt() << std::endl;
    int sz = stu["成绩"].size();
    for (int i = 0; i < sz; i++) {
        std::cout << "成绩: " <<  stu["成绩"][i].asFloat() << std::endl;
    }
    return 0;
}