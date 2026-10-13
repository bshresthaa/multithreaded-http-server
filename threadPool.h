// this needs to be a queue
//needs to hold  int. 
//Mutex to prevent syncronization issues.
//Condition variable to signal when a new task is added to the queue.

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "serverheader.h"

#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <thread>
#include <cstddef>
#include <fstream>

#include <chrono>
#include <thread>


const string response = 
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n";  

void handleClient(int connAccept) { 
        std::cout << "START socket " << connAccept
            << " thread " << std::this_thread::get_id()
            << std::endl;

        char buffer[1024] = {0};
        ssize_t bytesRead = readFromSocket(connAccept, buffer, sizeof(buffer));

        //parse the client request and extract the method and path.
        string requestString(buffer, bytesRead);  
        ssize_t firstSpace = requestString.find(" ");
        string method = requestString.substr(0, firstSpace);
        ssize_t secondSpace = requestString.find(" ", firstSpace+1);
        string path = requestString.substr(firstSpace+1, secondSpace-firstSpace-1);

        //Send response back
        if(method == "GET" && path == "/") { 
            cout<<"GET request received for /"<<endl;
                    //Open the file
            ifstream file("index.html");
            if(file.is_open()) { 
                cout<<"index.html file opened successfully."<<endl;
            }else {
                cout<<"Failed to open index.html file."<<endl;
            }

            //Read the files content
            string content = string(
                istreambuf_iterator<char>(file),
                istreambuf_iterator<char>()
            );
            int contentSize = content.size();
            string fileResponse = response + "Content-Length: " + to_string(contentSize) + "\r\n\r\n";
            fileResponse += content; 
                cout<<"\nSending response back to the client for GET and index path."<<endl;
            ssize_t sentData =  send(connAccept,fileResponse.c_str(), fileResponse.size(), 0);
            if(sentData == -1) { 
                perror("Sending response failed.");
            }else{ 
                cout<<"Response sent sucessfully."<<endl;
            }
        }else if(method == "GET" && path != "/") {
            path.erase(0, 1); // Remove the leading '/' from the path
            ifstream file(path); 

            if(!file.is_open()) { 
                cout<<"Failed to open file: "<<path<<endl;
                string notFoundResponse = 
                    "HTTP/1.1 404 Not Found\r\n"
                    "Content-Type: text/html\r\n"
                    "Content-Length: 48\r\n"
                    "\r\n"
                    "<html><body><h1>404 Not Found</h1></body></html>";
                send(connAccept, notFoundResponse.c_str(), notFoundResponse.size(), 0);
                close(connAccept);
                return; // Exit the function if the file is not found
            }

            string fileContent = string(
                istreambuf_iterator<char>(file),
                istreambuf_iterator<char>()
            );

            int contentSize = fileContent.size();
            string fileResponse = response + "Content-Length: " + to_string(contentSize) + "\r\n\r\n";
            fileResponse += fileContent; 
            cout<<"\nSending response back to the client for GET and file path."<<endl;
            ssize_t sentData =  send(connAccept,fileResponse.c_str(), fileResponse.size(), 0);
            if(sentData == -1) { 
                perror("Sending response failed.");
            }else{ 
                cout<<"Response sent sucessfully."<<endl;
            }
        }
        buffer[0] = '\0'; // Clear the buffer for the next

        std::cout << "END socket " << connAccept
          << " thread " << std::this_thread::get_id()
          << std::endl;

        close(connAccept);
}

class ThreadPool {
    private:
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

        void enqueueTask(int task) { 
            {
                std::lock_guard<std::mutex> lock(queueMutex);  //lock Mutex
                if(shutdown) { 
                    cout<< "ThreadPool is shutting down. Cannot enqueue new tasks."<<endl;
                    return ; 
                }
                else {
                    taskQueue.push(task); //enqueue task
                    cout<<"Task Enqueueed: "<<task<<endl;
                }
            }
            condition.notify_one();  //notify worker thread
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