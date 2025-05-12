#ifndef SAFEQUEUE_HPP
#define SAFEQUEUE_HPP
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <stdexcept>

template <typename T>
class safe_queue{
    /*
    *  线程安全的队列，用于线程池中任务的存储
    */
    private:
        std::queue<T> queue;
        std::mutex mutex;

    public:
        safe_queue(){}
        safe_queue(const safe_queue& other){
            std::lock_guard<std::mutex> lock(other.mutex);
            queue = other.queue;
        }
        ~safe_queue(){}

        bool empty(){
            /*
            * 判断返回队列是否为空
            * Args:
            *   None
            * Returns:
            *   bool: 返回队列是否为空
            */
            std::unique_lock<std::mutex> lock(mutex); //// 互斥信号变量加锁，防止queue被改变
            return queue.empty();
        }

        size_t size(){
            /*
            * 返回队列的大小
            * Args:
            *  None
            * Returns:
            *  size_t: 返回队列的大小
            */
            std::unique_lock<std::mutex> lock(mutex);
            return queue.size();
        }

        void enqueue(T& item){
            /*
            * 将item加入队列
            * Args:
            *   item: 待加入的元素
            * Returns:
            *   None
            */
            std::unique_lock<std::mutex> lock(mutex);
            queue.push(item);
        }

        bool dequeue(T& item){
            /*
            * 从队列中取出元素
            * Args:
            *   item: 取出的元素
            * Returns:
            *   bool: 是否成功取出元素
            */
            std::unique_lock<std::mutex> lock(mutex);
            if(queue.empty()) return false;
            item = std::move(queue.front()); 
            // 移动语义, 从队列中取出元素, 并将其赋值给item, 之后队列中的元素会被删除,可以减少内存的拷贝
            queue.pop();
            return true;
        }

        bool front(T& item){
            /*
            * 返回队列的第一个元素
            * Args:
            *   item: 返回的元素
            * Returns:
            *   bool: 是否成功返回元素
            */
            std::unique_lock<std::mutex> lock(mutex);
            if(queue.empty()) return false;
            item = queue.front();
            return true;
        }
};

#endif

