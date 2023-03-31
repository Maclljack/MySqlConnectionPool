#include"CommonConnectionPool.h"
#include"public.h"


//懒汉单例函数接口
ConnectionPool* ConnectionPool::getConnectionPool(){
    static ConnectionPool pool;
    return &pool;
}

//从配置文件中加载配置
bool ConnectionPool::loadConfigFile(){
    FILE *pf = fopen("connectionpool.cnf","r");
    if(nullptr == pf){
        LOG("connectionpool.cnf is not exist");
        return false;
    }

    while(!feof(pf)){
        char line[512] = {0};
        fgets(line, 512, pf);
        std::string str = line;
        int idx = str.find('=', 0);
        if(idx == -1){
            //无效的配置项
            continue;
        }

        int endidx = str.find('\n', idx);
        std::string key = str.substr(0, idx);
        std::string value = str.substr(idx+1, endidx - idx - 1);

        // std::cout<<"["<<key<<"] ["<<value<<"]"<<std::endl;
        if(key=="ip"){
            _ip = value;
        }else if (key=="port")
        {
            _port = atoi(value.c_str());
        }else if (key=="username")
        {
            _username = value;
        }else if (key=="password")
        {
            _password = value;
        }else if (key=="initSize")
        {
            _initSize = atoi(value.c_str());
        }else if (key=="maxSize")
        {
            _maxSize = atoi(value.c_str());
        }else if (key=="maxIdleTime")
        {
            _maxIdleTime = atoi(value.c_str());
        }else if (key=="connectionTimeout")
        {
            _connectionTimeout = atoi(value.c_str());
        }else if(key=="dbname"){
            _dbname = value;
        }else{

        }
        
    }
    fclose(pf);
    return true;
}

//构造函数
ConnectionPool::ConnectionPool(){
    _isDead = false;

    //加载配置项
    if(!loadConfigFile()){
        return;
    }

    //创建初始数量的连接
    for(int i = 0;i<_initSize;i++){
        Connection *p = new Connection();
        p->refreshAliveTime();//刷新起始空闲时间点
        p->connect(_ip, _port, _username, _password, _dbname);
        _connectionQue.push(p);
        // std::cout<<"new..."<<std::endl;
        _connectionCnt++;
    }

    //启动一个新的线程，作为连接的生产者
    std::thread produce(std::bind(&ConnectionPool::produceConnectionTask, this));
    produce.detach();

    //启动一个新的定时器线程，扫描超过maxIdleTime的空闲连接，将他们回收
    std::thread scanner(std::bind(&ConnectionPool::scannerConnectionTask, this));
    scanner.detach();
}

//析构函数
ConnectionPool::~ConnectionPool(){
    //将要销毁连接池
    _isDead = true;
    cv.notify_all();
    //睡三秒， 等待
    std::this_thread::sleep_for(std::chrono::seconds(3));

    //获取锁 准备释放资源
    std::unique_lock<std::mutex> loc(_queueMutex);

    while(!_connectionQue.empty()){
        Connection *p = _connectionQue.front();
        _connectionQue.pop();
        delete p;
        _connectionCnt--;
        // std::cout<<"delete..."<<std::endl;
}

}

//运行在一个新的线程中，专门生产新连接
void ConnectionPool::produceConnectionTask(){
    for(;;){
        std::unique_lock<std::mutex> loc(_queueMutex);
        while(!_connectionQue.empty()){
            cv.wait(loc);  //队列不空，生产者进入等待

            //连接池销毁 ==>此线程退出
            if(_isDead){
                return;
            }
        }
        if(_connectionCnt < _maxSize){
            //生产新的连接
            Connection *p = new Connection();
            p->refreshAliveTime();//刷新起始空闲时间点
            p->connect(_ip, _port, _username, _password, _dbname);
            _connectionQue.push(p);
            // std::cout<<"new..."<<std::endl;
            _connectionCnt++;
            
            
        }

        //通知消费者线程可以消费连接了
        cv.notify_all();
    }
}

//运行在一个新的线程中,扫描超过maxIdleTime的空闲连接，将他们回收
void ConnectionPool::scannerConnectionTask(){
    for(;;){
        //睡眠模仿定时效果
        std::this_thread::sleep_for(std::chrono::seconds(_maxIdleTime));
        //扫描队列，释放空闲连接
        std::unique_lock<std::mutex> loc(_queueMutex);
        while(_connectionCnt > _initSize){
            Connection *p = _connectionQue.front();
            if(p->getAliveTime()>=(_maxIdleTime*1000)){
                _connectionQue.pop();
                delete p;
                // std::cout<<"delete..."<<std::endl;
                _connectionCnt--;
            }else{
                break;
            }
        }
    }
    
}


//给外部提供接口，从连接池中获取一个可用的空闲连接
std::shared_ptr<Connection> ConnectionPool::getConnection(){
    std::unique_lock<std::mutex> loc(_queueMutex);
    while(_connectionQue.empty()){
        //连接队列为空
        if(std::cv_status::timeout == cv.wait_for(loc, std::chrono::milliseconds(_connectionTimeout))){
            //超时醒来发现还是空的
            if(_connectionQue.empty()){
                LOG("获取连接超时...");
                return nullptr;
            }
        }

    }


    //获取连接    需要自定义删除器，否则会释放掉connection, 应该把他归还到连接队列中
    std::shared_ptr<Connection> p(_connectionQue.front(),
        [&](Connection *con){
            std::unique_lock<std::mutex> loc(_queueMutex);
            con->refreshAliveTime();
            _connectionQue.push(con);
        });
    _connectionQue.pop();

    //通知生产者线程检查是否为空
    cv.notify_all();

    return p;
    

}