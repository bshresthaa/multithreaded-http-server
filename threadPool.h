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

using namespace std; 

const string response = 
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n";  

void handleClient(int connAccept) { 
        std::cout << "START socket " << connAccept
            << " thread " << std::this_thread::get_id()
            << std::endl;

        char buffer[1024] = {0};
        ssize_t bytesRead = readFromSocket(connAccept, buffer, sizeof(buffer));
        if (bytesRead <= 0) {
            close(connAccept);
            return;
        }
        //parse the client request and extract the method and path.
        string requestString(buffer, bytesRead);  
        size_t firstSpace = requestString.find(' ');
        if (firstSpace == string::npos) {
            close(connAccept);
            return;
        }

        size_t secondSpace = requestString.find(' ', firstSpace + 1);
        if (secondSpace == string::npos) {
            close(connAccept);
            return;
        }

        string method = requestString.substr(0, firstSpace);
        string path = requestString.substr(
            firstSpace + 1,
            secondSpace - firstSpace - 1
        );

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
        }else if (method == "GET" && path != "/") {
            // Remove the leading '/' from the path
            path.erase(0, 1);

            // Reject directory traversal attempts
            if (path.find("..") != string::npos) {
                string forbiddenResponse =
                    "HTTP/1.1 403 Forbidden\r\n"
                    "Content-Type: text/html\r\n"
                    "Content-Length: 48\r\n"
                    "Connection: close\r\n"
                    "\r\n"
                    "<html><body><h1>403 Forbidden</h1></body></html>";

                send(connAccept, forbiddenResponse.c_str(),
                    forbiddenResponse.size(), 0);

                close(connAccept);
                return;
            }

            // Open the requested file
            ifstream file(path);

            if (!file.is_open()) {
                cout << "Failed to open file: " << path << endl;

                string notFoundResponse =
                    "HTTP/1.1 404 Not Found\r\n"
                    "Content-Type: text/html\r\n"
                    "Content-Length: 48\r\n"
                    "Connection: close\r\n"
                    "\r\n"
                    "<html><body><h1>404 Not Found</h1></body></html>";

                send(connAccept, notFoundResponse.c_str(),
                    notFoundResponse.size(), 0);

                close(connAccept);
                return;
            }

            // Read the file contents
            string fileContent{
                istreambuf_iterator<char>(file),
                istreambuf_iterator<char>()
            };

            size_t contentSize = fileContent.size();

            string fileResponse =
                response +
                "Connection: close\r\n" +
                "Content-Length: " + to_string(contentSize) +
                "\r\n\r\n" +
                fileContent;

            cout << "Sending response for file: " << path << endl;

            ssize_t sentData = send(
                connAccept,
                fileResponse.c_str(),
                fileResponse.size(),
                0
            );

            if (sentData == -1) {
                perror("Sending response failed");
            } else if (static_cast<size_t>(sentData) < fileResponse.size()) {
                cerr << "Incomplete response sent." << endl;
            } else {
                cout << "Response sent successfully." << endl;
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
                    cout << "Socket queued for processing: " << task << endl;
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