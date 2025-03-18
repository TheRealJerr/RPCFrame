#include <iostream>

// muduo库

#include <muduo/net/TcpClient.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpConnection.h>
#include <muduo/net/Buffer.h>
#include <muduo/base/CountDownLatch.h>
#include <muduo/net/EventLoopThread.h>

class Client
{
public:
    Client(const std::string& sip,int port):
        baseloop_(thread_.startLoop()),
        countdownlatch_(1),
        client_(baseloop_,muduo::net::InetAddress(sip,port),"dict client")
    {
        // 设置回调函数
        client_.setConnectionCallback(std::bind(&Client::onConnection,this,std::placeholders::_1));
        client_.setMessageCallback(std::bind(&Client::onMessage,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
        // 连接服务器
        client_.connect();
        countdownlatch_.wait();// 等待connect
    }
    void send(const std::string& message)
    {
        if(conn_->connected() == false)
        {
            std::cout << "连接已经断开" << std::endl;
            return;
        }
        else 
            conn_->send(message);
    }
private:
void onConnection(const muduo::net::TcpConnectionPtr& conn)
{
    if(conn->connected())
    {
        std::cout << "连接建立成功" << std::endl;
        countdownlatch_.countDown();
        conn_ = conn;
    }
    else std::cout << "连接断开" << std::endl;
}

void onMessage(const muduo::net::TcpConnectionPtr& con,muduo::net::Buffer* buffer,muduo::Timestamp)
{
    std::string res = buffer->retrieveAllAsString();
    std::cout << res << std::endl;
}
private:
    muduo::net::TcpConnectionPtr conn_;
    muduo::net::EventLoopThread thread_;
    muduo::CountDownLatch countdownlatch_;
    muduo::net::EventLoop* baseloop_;
    muduo::net::TcpClient client_;
};

int main()
{
    Client clinet("127.0.0.1",8080);
    while(true)
    {
        std::string buffer;
        std::cin >> buffer;
        clinet.send(buffer);
        
    }
    return 0;
}