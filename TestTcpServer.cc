#include "TcpServer.h"
#include "TcpConnection.h"
#include "ThreadPool.h"
#include <iostream>

using std::cout;
using std::endl;

//全局变量
ThreadPool *gPool = nullptr;

class MyTask
{
public:
    MyTask(const string &msg, const TcpConnectionPtr &con)
    : _msg(msg)
    , _con(con)
    {

    }

    void process()
    {
        //所有的业务逻辑的处理，decode、compute、encode
        //在此处理msg1
        _msg;
        _msg += "hello";

        //线程池处理完毕业务逻辑之后，需要告诉Reactor/EventLoop去进行发送
        //数据，因为数据的收发不是线程池的功能，为了将职责分清楚，所以
        //Reactor/EventLoop进行IO操作，而线程池主要处理业务逻辑，也就是
        //进行计算操作，所以我们将IO操作的线程，称为IO线程，将计算操作的
        //线程，称为计算IO。
        //那么就会涉及到计算线程要通知IO线程进行数据的发送
        //那么就会涉及到线程之间的通信？
        //eventfd解决线程或者进程之间的通信。。。
        //将msg1发送数据给客户端
        _con->sendInLoop(_msg);
        //sendInLoop函数确实是TcpConnection的，但是消息的发送还是需要
        //Reactor，还是需要IO线程，还是需要EventLoop,所以TcpConnection
        //需要知道EventLoop的存在，不然处理之后的msg如何发送到
        //EventLoop，如何发送到Reactor，如何发送到IO线程
    }

private:
    string _msg;
    TcpConnectionPtr _con;
};

void onConnection(const TcpConnectionPtr &con)
{
    cout << con->toString() << " has connected!" << endl;
}

void onMessage(const TcpConnectionPtr &con)
{
    //接收客户端的数据
    //接收数据，也就是read数据，也就是读数据，是EventLoop线程
    //也就是Reactor线程，并且数据的发送也是属于EventLoop线程
    //或者Reactor线程
    string msg = con->receive();
    cout << ">>recv msg from client: " << msg << endl;

    //将接收的msg，业务逻辑进行decode compute encode操作
    //
    //
    MyTask task(msg, con);
    gPool->addTask(std::bind(&MyTask::process, task));//bind的地址传递与值传递
}

void onClose(const TcpConnectionPtr &con)
{
    cout << con->toString() << " has closed!" << endl;
}

int main(int argc, char **argv)
{
    ThreadPool pool(4, 10);
    pool.start();
    gPool = &pool;

    TcpServer server("127.0.0.1", 8888);
    server.setAllCallback(std::move(onConnection)
                          , std::move(onMessage)
                          , std::move(onClose));
    server.start();

    return 0;
}

