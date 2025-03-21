// 定义项目中交互的宏定义
#pragma once
#include <iostream>
#include <unordered_map>
#include <string>
namespace rpcframe
{
    // rps请求
    #define KEY_METHOD "mothod"
    #define KEY_PARAMS "parameter"
    // 主题操作请求
    #define KEY_TOP_KEY "topic_key"
    #define KEY_TOP_MSG "topic_msg"
    // 操作类型
    #define KEY_OP_TYPE "optype" 
    // 主机信息
    #define KEY_HOST "host"
    #define KEY_HOST_IP "ip"
    #define KEY_HOST_PORT "port"
    // 状态码
    #define KEY_RCODE "rcode"

    // req和req的类型  -- 根据我们的功能进行划分
    enum class Mtype
    {
        REQ_RPC = 0, 
        RSP_RPC,
        // 
        REQ_TOP,
        RSP_TOP,
        //
        REQ_SEVICE,
        RSP_SEVICE,
    };

    // 返回的状态码
    enum class RCode
    {
        RCODE_OK = 0, // 成功
        RCODE_PARSE_FAILED, // 解析失败
        RCODE_DISCONNECTED,  // 连接断开 
        RCODE_INVALID_MSG, // 无效的信息
        RCODE_INVALID_PARAMS,  
        RCODE_NOT_FIND_SEVIVE,
        RCODE_INVALID_OPTYPE,
        RCODE_NOT_FIND_TOPIC,
    };

    // 通过建立hash映射来进行映射错误
    static std::string errorCode(RCode code)
    {
        static std::unordered_map<RCode,std::string> err_map = 
        {
            { RCode::RCODE_OK, "成功处理" },
            { RCode::RCODE_PARSE_FAILED, "解析失败" },
            { RCode::RCODE_INVALID_MSG, "无效的信息" },
            { RCode::RCODE_DISCONNECTED, "连接断开" },
            { RCode::RCODE_INVALID_PARAMS, "无效参数" },
            { RCode::RCODE_NOT_FIND_SEVIVE, "没有找到rpc服务" },
            { RCode::RCODE_INVALID_OPTYPE, "无效操作类型" },
            { RCode::RCODE_NOT_FIND_TOPIC, "没有对应的主题" },
        };
        auto it = err_map.find(code);
        if(it == err_map.end()) return "未知错误";
        else return it->second;
    } 

    // RPC请求类型
    enum class RType
    {
        // 同步请求
        REQ_SYNC = 0,
        REQ_ASYNC,
        REQ_CALLBACK,
    };

    // 主题的操作类型定义
    enum class TopicOptType
    {
        TOPIC_CREATE = 0,
        TOPIC_REMOVE,
        TOPIC_SUBCRIBE,
        TOPIC_CANCEL,
        TOPIC_PUBLISH,
    };
    // 服务的上线和下线服务的注册和下线
    enum class ServiceOpType
    {
        SERVICE_REGISTY = 0,
        SERVICE_DISCOVERY,
        SERVICE_ONLINE,
        SERVICE_OFFLINE,
    };
}
