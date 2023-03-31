/*
实现连接池功能模块
*/
#ifndef COMMONCONNECTIONPOOL_H
#define COMMONCONNECTIONPOOL_H

#include"Connection.h"
#include<string>
#include<queue>
#include<mutex>
#include<condition_variable>
#include<atomic>
#include<memory>
#include<thread>
#include<functional>
#include<chrono>


class ConnectionPool
{
private:
    ConnectionPool(); //单例模式

    ~ConnectionPool(); //析构函数

    //从配置文件中加载配置
    bool loadConfigFile();

    //运行在一个新的线程中，专门生产新连接
    void produceConnectionTask();

    //扫描超过maxIdleTime的空闲连接，将他们回收
    void scannerConnectionTask();

    std::string _ip; //mysql的ip地址
    std::string _username; //mysql登录用户名
    unsigned short _port; //mysql端口号
    std::string _password; //mysql密码
    std::string _dbname; //连接的数据库名称
    int _initSize; //连接池初始连接量
    int _maxSize; //连接池最大连接量
    int _maxIdleTime; //连接池最大空闲时间 s
    int _connectionTimeout; //连接池获取连接的超时时间 ms

    std::queue<Connection*> _connectionQue; //存储mysql连接队列
    std::mutex _queueMutex; //维护队列线程安全的互斥锁
    std::atomic_int _connectionCnt; //记录连接所创建的connection的总数量
    std::condition_variable cv; //条件变量

    std::atomic_bool _isDead; //连接池是否销毁

public:
    //获取连接池对象实例
    static ConnectionPool* getConnectionPool();
   //给外部提供接口，从连接池中获取一个可用的空闲连接
    std::shared_ptr<Connection> getConnection();
};




#endif