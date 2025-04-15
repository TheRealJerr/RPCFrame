#pragma once

#include <iostream>
#include <string>
#include <unistd.h>
#include <unistd.h>
#include "/home/hrj/include/Mutex.hpp"
#include <fstream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <memory>
#include <sstream>
namespace LogModule
{
    // 1.构建日志文件
    const std::string gpath = "./log/";
    const std::string defaultlogname = "log.txt";

    // 为了实现向不同的文件进行写入这里我们使用多态的性质
    using namespace MutexModule;

    class LogPathBase
    {
    public:
        virtual void Write(const std::string &) = 0;
    }; //  声明为纯虚函数

    class LogPathFile : public LogPathBase
    {
    public:
        LogPathFile(const std::string &path = gpath, const std::string &filename = defaultlogname) : _path(path), _filename(filename)
        {
        }

        virtual void Write(const std::string &message) override
        {
            LockGuard lock(_mtx);

            if (!std::filesystem::exists(_path))
                std::filesystem::create_directories(_path);

            std::ofstream ofs(_path + _filename, std::ios::app); // 通过追加的方式进行写入

            if (ofs.is_open() == false)
                return;

            ofs << message << std::endl;
        }

    private:
        Mutex _mtx;

        const std::string _path;
        const std::string _filename;
    };

    class LogPathScreen : public LogPathBase
    {
        virtual void Write(const std::string &message) override
        {
            LockGuard lock(_mtx);
            std::cout << message << std::endl;
        }

    private:
        Mutex _mtx;
    };
    // 2. 日志等级

    enum class LogLevel
    {
        DEBUG,
        INFO,
        WARNING,
        ERROR,
        FATAL
    }; // 定义日志的等级
    std::string LevelToString(LogLevel level)
    {
        switch(level)
        {
            case LogLevel::DEBUG: return "Debug";
            case LogLevel::INFO: return "Info";
            case LogLevel::WARNING: return "Warning";
            case LogLevel::ERROR: return "Error";
            case LogLevel::FATAL: return "Fatal";
        }
        return "None";
    }
    // 首先确定日志的格式
    // 确定当前的时间
    std::string getCurtime()
    {
        auto now = std::chrono::system_clock::now();

        // 转换为time_t格式
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);

        // 格式化输出
        std::stringstream ssm;
        ssm << std::put_time(std::localtime(&now_time), "%Y-%m-%d %H:%M:%S");
        return ssm.str();
    }

    class Log
    {
    public:
        Log() : _src(std::make_unique<LogPathScreen>())
        {
        }

        void ChgToScreen() { _src = std::make_unique<LogPathScreen>(); }

        void ChgToFile() { _src = std::make_unique<LogPathFile>(); }

        // loginfo中存储的使我们想要输出的数据
        class LogInfo
        {
        public:
            LogInfo(LogLevel level,const std::string &filename,int line,Log& self):
                _level(level),
                _curtime(getCurtime()),
                _pid(::getpid()),
                _filename(filename),
                _line(line),
                _self(self)
            {
                std::stringstream ssm;
                ssm << '[' << _curtime << ']'\
                    << '[' << LevelToString(_level) << ']'\
                    << '[' << _filename << ']'\
                    << '[' << _line << ']'\
                    << '[' << _pid << ']';
                __log_all_info = std::move(ssm.str());
            }
            template <class Val>
            LogInfo &operator<<(const Val &arg)
            {
                std::stringstream ssm;
                ssm << arg;
                __log_all_info += ssm.str();// 将我们外部读取到的数据添加进入我们的__log_all_info
                return *this;
            }

            ~LogInfo()
            {
                _self._src->Write(__log_all_info);
            }
        private:
            LogLevel _level;
            std::string _curtime;
            pid_t _pid;
            std::string _filename;
            int _line;
            std::string __log_all_info;
            Log& _self;
        };
        LogInfo operator()(LogLevel level,std::string filename,int line)
        {
            return LogInfo(level,filename,line,*this);
        }

        
        
    private:
        std::unique_ptr<LogPathBase> _src;
    };
    // 3. 刷新策略
    Log log;
#define LOG(type) LogModule::log(type,__FILE__,__LINE__)
#define ENABLE_CONSOLE_LOG() log.ChgToScreen()
#define ENABLE_FILE_LOG() log.ChgToFile()


}