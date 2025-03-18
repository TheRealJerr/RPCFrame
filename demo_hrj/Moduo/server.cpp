/*
    实现一个翻译服务器，客户端发来一个英语单词，返回一个汉语
*/
#include <iostream>
#include <string>
#include <unordered_map>

#include <future>
// future::get获取结果  future.get如果获取不了会阻塞,
// muduo库
#include <muduo/net/TcpServer.h>
#include <muduo/net/TcpConnection.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/Buffer.h>



class Server
{
public:
    Server(int port):
        server_(&baseloop_,muduo::net::InetAddress("0.0.0.0",port),"dictionary",muduo::net::TcpServer::kReusePort)
    {   
        // 构建默认的字典
        dictionary_.insert({"hello","你好"});
        dictionary_.insert({"world","世界"});
        dictionary_.insert({"bit","比特"});
        // 设置回调函数
        server_.setConnectionCallback(std::bind(&Server::onConnection,this,std::placeholders::_1));
        server_.setMessageCallback(std::bind(&Server::onMessage,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
    }

    void start()
    {
        server_.start();// 开始监听
        baseloop_.loop(); // 开始事件循环
    }

private:
    void onConnection(const muduo::net::TcpConnectionPtr& conn)
    {
        if(conn->connected())
        {
            std::cout << "连接建立成功" << std::endl;   
        }
        else 
        {
            std::cout << "连接断开" << std::endl;
        }
    }

    void onMessage(const muduo::net::TcpConnectionPtr& con,muduo::net::Buffer* buffer,muduo::Timestamp)
    {
        std::string send_message;

        std::string message = buffer->retrieveAllAsString();
        if(dictionary_.count(message))
            send_message = dictionary_[message];
        else 
            send_message = "乱输入";
        con->send(send_message); 
    }
private:
    muduo::net::EventLoop baseloop_;

    muduo::net::TcpServer server_;
    std::unordered_map<std::string,std::string> dictionary_;
};

int main()
{
    Server server(8080);
    server.start();
    return 0;
}