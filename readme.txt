已完成
1 网络库:山寨muduo，主要是封装tcp socket epoll，主动连接方有一定重试机制。
2 log:实现了一个简单的，z在加log4cxx
3 framework:实现了简单的proc流程，config，tcp control等等功能
4 timer：简单做了一个，可以考虑是否用时间轮做个更高效的。
5 协程：简单实现了一个
6 容器: 做了一个对象池，其他用stl够了
7 数据描述：全部用pb

计划中工作
1 通过protobuf生成sql语句表，包括create和alert
2 接入服务器：
3 db服务器：



纠结中
1 服务器之间的通讯是否要引入一层通讯代理层，屏蔽后端细节，容灾+负载均衡
2 是否支持脚本扩展




参考
rpc: 考虑是否有必要做
zookeeper：自动部署，容灾，分布式管理
perftools：代码内置profiling工具
http out：输出http管理界面
监控 ganglia
运维 puppet saltstack Fabric Ansible 
libzdb: mysql一个并发库
