# mogujie Open Source IM App  ![Logo](https://avatars2.githubusercontent.com/u/8542441?v=2&s=200)

[mogujie](http://www.mogujie.com) Open Source IM is aiming to provide an another IM solution in your company for colleagues to communicate with each other. 

we've released Win/Mac/Android/iOS  client repositories in github as well as IM server repository.

see all projects in our [mogutt](https://github.com/mogutt) github account page or visit our [official website](http://tt.mogu.io/)(Chinease) for more information

## Windows Client Features
* list all colleagues in your company as well as detail profiles like (email addr, title, phone No. etc.)
* support fast search colleague detail profile
* support communicating through "Text", "Audio", "Image" messages like [whatsapp](http://www.whatsapp.com/) 
* support creating temporary chat group with multiple people all together
* support permanent chat group created by administrators
* support basic functions such as Emotion、Vibrating screen、File transfer and so on

# 编译
* 开发工具：VS2013
* 在TTWinClient同一级目录下clone TTServer工程https://github.com/mogutt/TTServer
* 工程文件放在TTWinClient\solution\下

# 使用
* 1 为了使客户端开发人员能够不需要部署服务器直接跑起来调试，在启动之后跳出服务器地址配置窗口如图：![服务器地址配置窗口](http://s7.mogucdn.com/b7/pic/141011/8dxwb_ieygmmjymm3dinlemmytambqgiyde_300x120.jpg_468x468.jpg)输入http://alpha.mogu.io/
* 2 在登陆窗口输入 用户名：小李或小张  密码都是：123456

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
    
    
