// 项目中的琐碎功能
// Common接口
#pragma once

#include <iostream>
#include <jsoncpp/json/json.h>
#include <ctime>
#include <cstdio>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <random>
#include <mutex>
#include <cstring>
#include <iomanip>
#include <atomic>

// 1. 日志宏的定义

#define LDBG 0
#define LINF 1
#define LERR 2

#define LDFAULT LDBG

std::mutex global_log_lock;

#define LOG(level, format, ...)                                                                 \
    {                                                                                           \
        std::unique_lock<std::mutex>                                                            \
            lock(global_log_lock);                                                              \
        if (level >= LDFAULT)                                                                   \
        {                                                                                       \
            time_t current_time;                                                                \
            time(&current_time);                                                                \
            struct tm *curtime = localtime(&current_time);                                      \
            char format_str[32] = {0};                                                          \
            strftime(format_str, 31, "%m-%d %T", curtime);                                      \
            printf("[%s][%s:%d]: " format "\n", format_str, __FILE__, __LINE__, ##__VA_ARGS__); \
        }                                                                                       \
    }

#define DLOG(format, ...) LOG(LDBG, format, ##__VA_ARGS__)
#define ILOG(format, ...) LOG(LINF, format, ##__VA_ARGS__)
#define ELOG(format, ...) LOG(LERR, format, ##__VA_ARGS__)

// 2. json的序列化和反序列化

namespace rpcframe
{
    class JsonTools
    {
    public:
        static bool serialize(const Json::Value &val, std::string &body)
        {
            Json::StreamWriterBuilder factory;
            std::unique_ptr<Json::StreamWriter> writer(factory.newStreamWriter());
            std::stringstream ssm;
            int n = writer->write(val, &ssm);

            if (n != 0)
            {
                ELOG("serialize error:");
                return false;
            }

            body = std::move(ssm.str()); // 直接将右值给他
            return true;
        }
        // 反序列化
        static bool deserialize(Json::Value &val, const std::string &body)
        {
            Json::CharReaderBuilder factory;
            std::unique_ptr<Json::CharReader> reader(factory.newCharReader());
            std::string errors;
            bool ret = reader->parse(body.c_str(), body.c_str() + body.size(), &val, &errors);
            if (ret == false)
            {
                // std::cout << "deserialize fail:" << errors << std::endl;
                ELOG("deserialize fail: %s", errors.c_str());
                return false;
            }
            else
            {
                return true;
            }
        }
    };
}

// 3. uuid的实现

// 生成随机数
// uuid生成器

namespace rpcframe
{
    class UUIDTool
    {
    public:
        static std::string getUUID()
        {
            std::stringstream ssm;
            std::random_device rd;
            std::mt19937 generator(rd());
            std::uniform_int_distribution<int> distribution(0, 255);

            for (int i = 0; i < 8; i++)
            {
                if (i == 4 || i == 6)
                    ssm << "-";
                ssm << std::setw(2) << std::setfill('0') << std::hex << distribution(generator);
            }
            ssm << "-";
            static std::atomic<size_t> seq(1);
            size_t cur = seq.fetch_add(1);
            for (int i = 7; i >= 0; i--)
            {
                if (i == 5)
                    ssm << "-";
                ssm << std::setw(2) << std::setfill('0') << std::hex << ((cur >> i * 8) & 0xFF);
            }
            return ssm.str();
        }
    };
}
