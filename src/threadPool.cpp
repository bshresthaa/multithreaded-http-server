// this needs to be a queue
//needs to hold  int. 
//Mutex to prevent syncronization issues.
//Condition variable to signal when a new task is added to the queue.
#include "threadPool.h"
#include "httpHandler.h"
#include <utility>

ThreadPool::ThreadPool(std::size_t numThreads) {
    for(std::size_t i =0; i<numThreads; ++i) { 
        //workerThread() is a non-static member function. It belongs to a particular ThreadPool object
        workerThreads.emplace_back(&ThreadPool::workerThread, this); 
    }
};

ThreadPool::~ThreadPool() {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        shutdown = true;  
    } //Mutex is realeased 
    condition.notify_all();  //notify all worker threads to exit
    //Join the threads
    for(std::thread &worker : workerThreads) { 
        if(worker.joinable()) { 
            worker.join(); //wait for the thread to finish
        }
    }
}; 

bool ThreadPool::enqueueTask(Socket&& task) { 
    {
        std::lock_guard<std::mutex> lock(queueMutex);  //lock Mutex
        if(shutdown) { 
            std::cout<< "ThreadPool is shutting down. Cannot enqueue new tasks."<<std::endl;
            return false; 
        }
        
        if(taskQueue.size() >= maxQueueSize) {
            std::cout << "Task queue is full. Cannot enqueue new task: " << std::endl;
            return false;
        }

        taskQueue.push(std::move(task)); //enqueue task
        std::cout << "Socket queued for processing: " << std::endl;
    }
    condition.notify_one();  //notify worker thread
    return true; 
}

void ThreadPool::workerThread() { 
    std::cout << "Worker started: "
    << std::this_thread::get_id() << std::endl;

    while(true) { 
        std::unique_lock<std::mutex> lock(queueMutex); 
        condition.wait(
            lock, [this] { 
                return !taskQueue.empty() || shutdown; 
            }
        );
        if(taskQueue.empty() && shutdown) { 
            return; 
        }  
        Socket task = std::move(taskQueue.front());
        taskQueue.pop(); 
        lock.unlock();

    std::cout << "Worker "
    << std::this_thread::get_id()
    << " handling socket " << std::endl;

        handleClient(task); //process the task
    }
}

