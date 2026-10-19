// this needs to be a queue
//needs to hold  int. 
//Mutex to prevent syncronization issues.
//Condition variable to signal when a new task is added to the queue.

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include  "httphandler.h"


class ThreadPool {
    private:
        const int maxQueueSize = 10; // Maximum number of tasks in the queue
        
        std::queue<int> taskQueue;
        std::mutex queueMutex; 
        std::condition_variable condition; 
        std::vector<std::thread> workerThreads;
        bool shutdown = false;  

    public:
        ThreadPool(std::size_t numThreads) {
            for(std::size_t i =0; i<numThreads; ++i) { 
                //workerThread() is a non-static member function. It belongs to a particular ThreadPool object
                workerThreads.emplace_back(&ThreadPool::workerThread, this); 
            }
        };

        ~ThreadPool() {
            {
                lock_guard<std::mutex> lock(queueMutex);
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

        bool enqueueTask(int task) { 
            {
                std::lock_guard<std::mutex> lock(queueMutex);  //lock Mutex
                if(shutdown) { 
                    cout<< "ThreadPool is shutting down. Cannot enqueue new tasks."<<endl;
                    return false; 
                }
                else {
                    if(taskQueue.size() >= maxQueueSize) {
                        cout << "Task queue is full. Cannot enqueue new task: " << task << endl;
                        return false;
                    }
                    taskQueue.push(task); //enqueue task
                    cout << "Socket queued for processing: " << task << endl;
                }
            }
            condition.notify_one();  //notify worker thread
            return true; 
        }

        void workerThread() { 
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
                int task = taskQueue.front();
                taskQueue.pop(); 
                lock.unlock();

            std::cout << "Worker "
            << std::this_thread::get_id()
            << " handling socket " << task << std::endl;

                handleClient(task); //process the task
            }
        }
}; 

#endif