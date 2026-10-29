// this needs to be a queue
//needs to hold  int. 
//Mutex to prevent syncronization issues.
//Condition variable to signal when a new task is added to the queue.

#pragma once

#include "httpHandler.h"

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <cstddef>
#include <iostream>


class ThreadPool {
    private:

        const std::size_t maxQueueSize = 10; // Maximum number of tasks in the queue
        
        std::queue<int> taskQueue;
        std::mutex queueMutex; 
        std::condition_variable condition; 
        std::vector<std::thread> workerThreads;
        bool shutdown = false;  

        void workerThread();
        
    public:

        explicit ThreadPool(std::size_t numThreads);
        ~ThreadPool(); 

        ThreadPool(const ThreadPool&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;

        bool enqueueTask(int task);

};
