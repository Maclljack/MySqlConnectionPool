/*
实现MySql数据库的增删改查操作

*/
#ifndef CONNECTION_H
#define CONNECTION_H

#include<string>
#include<mysql/mysql.h>
#include<ctime>

class Connection{
public:
    //初始化数据库连接
    Connection();
    //释放数据库连接
    ~Connection();
    //连接数据库
    bool connect(
        std::string ip,
        unsigned short port,
        std::string user,
        std::string password,
        std::string dbname
    );

    //更新操作 insert, delete,uqdate
    bool update(std::string sql);

    //查询操作
    MYSQL_RES* query(std::string sql);

    //刷新连接的起始空闲时间点
    void refreshAliveTime(){_alivetime = clock();}
    //返回存活时间 单位毫秒
    clock_t getAliveTime(){ return (clock()-_alivetime)/(CLOCKS_PER_SEC/1000);}
    
private:
    MYSQL *_conn;
    clock_t _alivetime; //记录进入空闲状态后的存活时间

};


#endif