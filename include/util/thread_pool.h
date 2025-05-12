#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <stdexcept>
#include "safe_queue.h"
class ThreadPool{
    /*
    * 线程池类，用于多线程任务提交，任务队列管理
    */
    private:
        std::vector<std::thread> workers;           // 工作线程列表
        safe_queue<std::function<void()>> tasks;     // 任务队列，存储可调用对象
        bool stop;                                  // 停止标志，用于指示线程池是否应该停止
        std::mutex mutex;                           // 互斥锁
        std::condition_variable condition;          // 条件变量，用于等待任务
        std::vector<std::atomic<bool>> finished;    // 用于判断线程是否完成任务
        std::mutex finished_mutex;                  // 用于finished的互斥锁

    public:

    ThreadPool(int nThreads = 4): workers(std::vector<std::thread>(nThreads)),finished(nThreads),stop(false){
        /*
        * 初始化线程池
        * Args:
        *  nThreads: 线程池中线程的数量
        */
        for (int i = 0; i < nThreads; i++){
            workers.at(i) = std::thread([this, i]{
                while(!this->stop){
                    std::function<void()> task;
                    {
                        // 同步锁，防止线程空转
                        std::unique_lock<std::mutex> lock(this->mutex);
                        this->condition.wait(lock, [this]{return this->stop || !this->tasks.empty();});
                        std::unique_lock<std::mutex> lock_finish(this->finished_mutex);
                        this->finished.at(i).store(false);
                        lock_finish.unlock();
                        if(this->stop && this->tasks.empty()) return;
                        this->tasks.dequeue(task);
                    }
                    task();
                    std::unique_lock<std::mutex> lock_finish(this->finished_mutex);
                    this->finished.at(i).store(true);
                    lock_finish.unlock();
                }
            });
        }
    }

    ThreadPool(const ThreadPool& other) = delete; // 禁用拷贝构造函数
    ThreadPool(ThreadPool&& other) = delete;  // 禁止赋值操作符
    ThreadPool& operator=(const ThreadPool& other) = delete;
    ThreadPool& operator=(ThreadPool&& other) = delete;

    // 析构函数，关闭线程池
    ~ThreadPool(){shutdown();}

    void shutdown(){
        /*
        * 关闭线程池
        */
        stop = true;
        condition.notify_all();
        for(auto& worker: workers)
            if (worker.joinable()) worker.join();
    }

    bool hasFinished(){
        /*
        * 判断线程池是否完成任务
        *   Returns:
        *      bool: 返回线程池是否完成任务
        */
        // for (auto& worker: workers){
        //     if (worker.joinable()) return false;
        // }
        // return true;
        return this->tasks.empty() && std::all_of(finished.begin(), finished.end(), [](bool x){return x;});
    }
    
    template <typename F, typename... Args>
    auto submit(F &&f, Args &&...args) -> std::future<decltype(f(args...))>{
        /*
        * 提交任务到线程池
        * Args:
        *  f: 函数
        *  args: 函数参数
        * Returns:
        *  std::future<decltype(f(args...))>: 返回函数的返回值
        */
        // 创建一个函数，将函数和参数绑定
        std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...); // 连接函数和参数定义，特殊函数类型，避免左右值错误
        // 创建一个任务指针，用于获取函数的返回值
        auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);
        // 创建一个安全封包函数，用于将任务指针压入安全队列
        std::function<void()> warpper_func = [task_ptr](){
            (*task_ptr)();
        };
        // 队列通用安全封包函数，并压入安全队列
        tasks.enqueue(warpper_func);
        // 唤醒一个等待中的线程
        condition.notify_one();
        // 返回先前注册的任务指针
        return task_ptr->get_future();
    }
};
#endif