#ifndef SAFEVECTOR_HPP
#define SAFEVECTOR_HPP
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <stdexcept>

template <typename T>
class safe_vector{
    /*
    * 线程安全的vector，用于线程池中任务的存储
    */
    private:
        std::vector<T> vec;
        std::mutex mutex;

    public:
        safe_vector(){}
        safe_vector(const safe_vector& other){
            std::lock_guard<std::mutex> lock(other.mutex);
            vec = other.vec;
        }
        ~safe_vector(){}

        bool empty(){
            /*
            * 判断返回是否为空
            * Args:
            *   None
            * Returns:
            *   bool: 返回是否为空
            */
            std::unique_lock<std::mutex> lock(mutex);
            return vec.empty();
        }

        size_t size(){
            /*
            * 返回大小
            * Args:
            *  None
            * Returns:
            *  size_t: 返回大小
            */
            std::unique_lock<std::mutex> lock(mutex);
            return vec.size();
        }

        void push_back(T& item){
            /*
            * 将item加入尾部
            * Args:
            *   item: 待加入的元素
            * Returns:
            *   None
            */
            std::unique_lock<std::mutex> lock(mutex);
            vec.push_back(item);
        }
        void push_back(T&& item){
            /*
            * 将item加入尾部
            * Args:
            *   item: 待加入的元素
            * Returns:
            *   None
            */
            std::unique_lock<std::mutex> lock(mutex);
            vec.push_back(std::move(item));
        }
        bool pop_back(T& item){
            /*
            * 从取出尾部元素
            * Args:
            *   item: 取出的元素
            * Returns:
            *   bool: 是否成功取出元素
            */
            std::unique_lock<std::mutex> lock(mutex);
            if(vec.empty()){
                return false;
            }
            item = vec.back();
            vec.pop_back();
            return true;
        }
        bool front(T& item){
            /*
            * 返回第一个元素
            * Args:
            *   item: 返回的元素
            * Returns:
            *   bool: 是否成功返回元素
            */
            std::unique_lock<std::mutex> lock(mutex);
            if(vec.empty()){
                return false;
            }
            item = vec.front();
            return true;
        }
        T& operator[](size_t index){
            /*
            * 下标运算符重载
            * Args:
            *   index: 下标
            * Returns:
            *   T&: 返回下标对应的元素
            */
            std::unique_lock<std::mutex> lock(mutex);
            if(index >= vec.size()){
                throw std::out_of_range("Index out of range");
            }
            return vec[index];
        }
        void clear(){
            /*
            * 清空数据
            * Args:
            *   None
            * Returns:
            *   None
            */
            std::unique_lock<std::mutex> lock(mutex);
            vec.clear();
        }
        void resize(size_t size){
            /*
            * 重置大小
            * Args:
            *   size: 新的大小
            * Returns:
            *   None
            */
            std::unique_lock<std::mutex> lock(mutex);
            vec.resize(size);
        }
        void reverse(){
            /*
            * 反转数据
            * Args:
            *   None
            * Returns:
            *   None
            */
            std::unique_lock<std::mutex> lock(mutex);
            std::reverse(vec.begin(), vec.end());
        }
        T& at(size_t index){
            /*
            * 返回指定位置的元素
            * Args:
            *   index: 下标
            * Returns:
            *   T&: 返回下标对应的元素
            */
            std::unique_lock<std::mutex> lock(mutex);
            if(index >= vec.size()){
                throw std::out_of_range("Index out of range");
            }
            return vec[index];
        }
};

#endif