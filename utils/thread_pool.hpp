//
// Created by Yongliang Yang on 2023/5/12.
//

#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <mutex>
#include <queue>
#include <functional>
#include <future>
#include <thread>
#include <utility>
#include <vector>


// Thread safe implementation of a Queue using a std::queue
template <typename T>
class SafeQueue
{
private:
    std::queue<T> _queue;  //工作队列
    std::mutex _mutex;  //互斥信号量

public:
    SafeQueue() {}
    SafeQueue(SafeQueue &&safeQueue) {}
    ~SafeQueue() {}

    bool empty()
    {
        //unique_lock的基本作用，在构造时加锁，析构时解锁
        std::unique_lock<std::mutex> lock(_mutex);
        return _queue.empty();
    }

    int size()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        return _queue.size();
    }

    void enqueue(T &task)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _queue.emplace(task);
    }

    bool dequeue(T &task)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        if (_queue.empty()) return false;

        // 移动语义，交出对象的控制权，并将队列最前面的元素弹出
        task = std::move(_queue.front());
        _queue.pop();

        return true;
    }
};


class ThreadPool
{
private:
    class ThreadWorker
    {
    private:
        int _id;
        ThreadPool* _pool;
    public:
        ThreadWorker(ThreadPool* pool, const int& id) : _pool(pool), _id(id) {}

        void operator()(){
            std::function<void()> func;
            bool is_dequeue;
            while(!_pool->_shutdown)
            {
                {
                    std::unique_lock<std::mutex> lock(_pool->_condition_mutex);
                    if (_pool->_queue.empty())
                    {
                        _pool->_condition_lock.wait(lock);
                    }
                    is_dequeue = _pool->_queue.dequeue(func);
                }
                if (is_dequeue) func();
            }
        }
    };

    bool _shutdown;
    SafeQueue<std::function<void()>> _queue;
    std::vector<std::thread> _thread;
    std::mutex _condition_mutex;
    std::condition_variable _condition_lock;


public:
    explicit ThreadPool(const int n_threads = 4) : _thread(std::vector<std::thread>(n_threads)), _shutdown(false) {}

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool(const ThreadPool&&) = delete;
    ThreadPool &operator=(const ThreadPool &) = delete;
    ThreadPool &operator=(ThreadPool &&) = delete;

    void init()
    {
        for (int i = 0; i < _thread.size(); ++i) {
            _thread[i] = std::thread(ThreadWorker(this, i));
        }
    }

    void shutdown()
    {
        _shutdown = true;
        _condition_lock.notify_all();
        for (int i = 0; i < _thread.size(); ++i) {
            if (_thread[i].joinable()) _thread[i].join();
        }
    }

    template<typename F, typename... Args>
    auto submit(F &&f, Args &&...args) -> std::future<decltype(f(args...))>
    {
        // 将参数与执行函数绑定，同时对参数使用完美转发，保证实参的信息
        std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...)())>>(func);

        std::function<void()> warpper_func = [task_ptr]() {
            (*task_ptr)();
        };

        _queue.enqueue(warpper_func);
        _condition_lock.notify_all();
        return task_ptr->get_future();
    }
};



#endif

