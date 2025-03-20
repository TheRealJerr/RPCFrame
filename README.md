# RPCFrame

c++实现的基于jsoncpp的RPC框架

## 项目的三个主要功能

* rpc调用:

* 服务的注册与发现以及服务的下线/上线通知

* 消息的发布订阅

## 服务端功能模块

* 基于网络通信接受客户端的请求,提供rpc服务

* 服务注册于发现,上下线通知

* 提供主题操作(创建/删除/订阅/取消)

### NetWork

基于moduo库实现网络通信

### Dispatch

|---|---|
|消息类型|回调函数|
|type|handler|

### protocal

/Lenth/Mtype/Id(unique)/