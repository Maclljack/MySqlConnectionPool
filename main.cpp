#include"Connection.h"
#include<iostream>
#include"CommonConnectionPool.h"

#include<ctime>

int main(){
    // Connection cn;
    // cn.connect("192.168.23.131", 3306, "root", "123456", "mytest1");
    // // cn.update("create table te1(sno INT PRIMARY KEY, sname VARCHAR(4));");
    // char sql[1024] = {0};
    // sprintf(sql, "insert into t1(sname, sage) values('%s',%d)","xi444", 18);
    // if(cn.update(sql)){
    //     std::cout<<"插入成功"<<std::endl;
    // }

    ConnectionPool * pool = ConnectionPool::getConnectionPool();
    
    clock_t begin = clock();
    for(int i = 0;i<1000;i++){

        // std::shared_ptr<Connection> cn(pool->getConnection());

        // // cn.update("create table te1(sno INT PRIMARY KEY, sname VARCHAR(4));");
        // char sql[1024] = {0};
        // sprintf(sql, "insert into t1(sname, sage) values('%s',%d)","xi444", 18);
        // cn->update(sql);



    Connection cn;
    cn.connect("192.168.23.131", 3306, "root", "123456", "mytest1");
    // cn.update("create table te1(sno INT PRIMARY KEY, sname VARCHAR(4));");
    char sql[1024] = {0};
    sprintf(sql, "insert into t1(sname, sage) values('%s',%d)","xi444", 18);
    cn.query(sql);
        
    }

    clock_t end = clock();
    std::cout<<"用时: "<<(end-begin)/(1.0*CLOCKS_PER_SEC)<<" s"<<std::endl;


    // auto result = cn.query("select * from t1");
    // MYSQL_ROW row ;
    // unsigned int num_fields;
    // unsigned int i;
    // num_fields = mysql_num_fields(result);
    // while ((row = mysql_fetch_row(result)))
    // {
    //     unsigned long *lengths;//数组指针 当前行的每一列的长度
    //     lengths = mysql_fetch_lengths(result);
    //     for(i = 0; i < num_fields; i++)
    //     {
    //         printf("%ld...",lengths[i]);
    //         printf("[%s] ",row[i]);
    //     }
    //     printf("\n");
    // }
    // mysql_free_result(result);



    return 0;
}