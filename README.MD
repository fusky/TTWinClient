# 简介
* TeamTalk是一套开源的企业办公即时通讯软件，作为整套系统的组成部分之一，TTWinClient为TeamTalk 的windows IM客户端，为用户提供收发文本消息，图片，表情等功能。

# 编译
* 开发工具：VS2013
* 在TTWinClient同一级目录下clone TTServer工程https://github.com/mogutt/TTServer
* 工程文件放在TTWinClient\solution\下

# 使用
* 1 为了使客户端开发人员能够不需要部署服务器直接跑起来调试，在启动之后跳出服务器地址配置窗口如图：![服务器地址配置窗口](http://s7.mogucdn.com/b7/pic/141011/8dxwb_ieygmmjymm3dinlemmytambqgiyde_300x120.jpg_468x468.jpg)输入http://alpha.mogu.io/
* 2 在登陆窗口输入用户名：小李  密码：123456

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
    
    
