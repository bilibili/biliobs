# biliobs
### 直播姬基于或引用了以下项目
+ [Qt5](http://www.qt.io/)
+ [obs-studio](https://github.com/jp9000/obs-studio)
+ [openssl](https://github.com/openssl/openssl)
+ [ffmpeg](https://git.ffmpeg.org/ffmpeg.git)
+ [cURL](https://github.com/curl/curl)
+ [jansson](https://github.com/akheron/jansson)
+ [boost](http://www.boost.org/)
+ [netbsd](https://www.netbsd.org/)

#### add by moruoxian
2020 04 
基于哔哩哔哩分支 
作者moruoxian开发环境 vs2019 + qt 5.14.1 

由于官方提供的login.dll 可能 存在用户最低版本验证
以及相关的 jansson 和 libcurl 等第三方c库的升级 修改了部分冲定义的头文件  移除了 login.dll
相关逻辑 但是 依旧会产生空指针现象 但是主界面是可以显示的 哔哩哔哩 代码值得研究 
其代码中大量应用 窗体的工厂类 来根据 来源的不同 来生产不同窗体的函数指针 从而打开各种属性窗体 。
 
 界面布局的风格值得学习 
 
 20200507 
 持续跟进中 哔哩哔哩 中的 核心功能 添加 场景 和 来源 都是基于obs 中的api 来完成的
 例如obs_data()  是 jason 结构中的最小单位。根据相关的界面操作来 创建 临时或者持久化的 json 文件到 哔哩哔哩制定的 data 路径下来完成操作源的增加或更新 。本人将持续跟进该项目研究。
 
 20200507 
 将部分ui继承 到了 obs-studio 自己的fork分支 https://github.com/moruoxian/obs-studio/tree/AddBiliUI 中
