# 简介
* TeamTalk是一套开源的企业办公即时通讯软件，作为整套系统的组成部分之一，TTWinClient为TeamTalk 的windows IM客户端，为用户提供收发文本消息，图片，表情等功能

# 当前支持的功能点
* 私人聊天
* 群组聊天
* 文件传输
* 多点登录
* 组织架构

# 工程模块框架图
![win TT框架图](http://s8.mogucdn.com/b7/pic/140928/nb8ca_ieydonjsge2tmmrzmmytambqgiyde_803x546.jpg)
####介绍
    win TT项目总共分为四个层次:
    * 第一层基础类库
    * 第二层逻辑框架
    * 第三层业务模块层
    * 第四层teamtalk项目控制层
####基础类库：
    * utility:一些公用的基础函数库，如字符串处理、数据库读写、windows线程库包装等
    * net:网络处理基础类库，包括httpclient、异步I/O socket的包装
    * TTProject：目前提供dump收集的保护
    * duilib：windowless ui开源库
####TTLogic：
    * TTLogic：一套支撑win TT逻辑框架,包括operation任务调度、事件的订阅与发布、基于WSAAsyncSelect模型的网络异步I/O TCP/IP长连接、MKO等，详细设计文档正在完善中...
####Modules:
    * FileTransfer：文件传输
    * Capture：截屏
    * Session：个人会话/群会话管理
    * Login：登陆和断线重连的管理
    * UserList：组织架构和用户列表信息的管理
    * Message：会话消息收发的管理
####TeamTalk
    * TeamTalk：整个项目工程控制的一些业务
    
    
