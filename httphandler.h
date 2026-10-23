#pragma once

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

bool sendAll(int socket, const string& data) {
    size_t totalSent = 0;

    while (totalSent < data.size()) {
        ssize_t sent = send(socket,data.c_str() + totalSent,data.size() - totalSent,0);
        if (sent <= 0) {
            perror("send failed");
            return false;
        }
        totalSent += static_cast<size_t>(sent);
    }

    return true;
}                

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
            ifstream file("public/index.html");

            if (!file.is_open()) {
                string notFoundResponse =
                    "HTTP/1.1 404 Not Found\r\n"
                    "Content-Length: 0\r\n"
                    "Connection: close\r\n"
                    "\r\n"; 
                sendAll(connAccept, notFoundResponse);                
                close(connAccept);
                return;
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
            if (!sendAll(connAccept, fileResponse)) {
                cerr << "Failed to send complete response." << endl;
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

                sendAll(connAccept, forbiddenResponse);

                close(connAccept);
                return;
            }

            // Open the requested file
            ifstream file("public/" + path);

            if (!file.is_open()) {
                cout << "Failed to open file: " << path << endl;

                string notFoundResponse =
                    "HTTP/1.1 404 Not Found\r\n"
                    "Content-Type: text/html\r\n"
                    "Content-Length: 48\r\n"
                    "Connection: close\r\n"
                    "\r\n"
                    "<html><body><h1>404 Not Found</h1></body></html>";

                sendAll(connAccept, notFoundResponse);

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

            //Check for partial send and handle it
            if (!sendAll(connAccept, fileResponse)) {
                cerr << "Failed to send complete response." << endl;
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
