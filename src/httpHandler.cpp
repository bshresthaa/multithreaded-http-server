#include "httpHandler.h"
#include "serverHeader.h"

#include <cstddef>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

static const std::string response = 
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n";  

bool sendAll(int socket, const std::string& data) {
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

void handleClient(Socket&  client) { 
        int connAccept = client.get();
        std::cout << "START socket " << connAccept
            << " thread " << std::this_thread::get_id()
            << std::endl;

        char buffer[1024] = {0};
        ssize_t bytesRead = readFromSocket(connAccept, buffer, sizeof(buffer));
        if (bytesRead <= 0) {
            return;
        }
        //parse the client request and extract the method and path.
        std::string requestString(buffer, bytesRead);  
        size_t firstSpace = requestString.find(' ');
        if (firstSpace == std::string::npos) {
            return;
        }

        size_t secondSpace = requestString.find(' ', firstSpace + 1);
        if (secondSpace == std::string::npos) {
            return;
        }

        std::string method = requestString.substr(0, firstSpace);
        std::string path = requestString.substr(
            firstSpace + 1,
            secondSpace - firstSpace - 1
        );

        //Send response back
        if(method == "GET" && path == "/") { 
            std::cout<<"GET request received for /"<<std::endl;
                    //Open the file
            std::ifstream file("public/index.html");

            if (!file.is_open()) {
                std::string notFoundResponse =
                    "HTTP/1.1 404 Not Found\r\n"
                    "Content-Length: 0\r\n"
                    "Connection: close\r\n"
                    "\r\n"; 
                sendAll(connAccept, notFoundResponse);                
                return;
            }

            //Read the files content
            std::string content = std::string(
                std::istreambuf_iterator<char>(file),
                std::istreambuf_iterator<char>()
            );
            int contentSize = content.size();
            std::string fileResponse = response + "Content-Length: " + std::to_string(contentSize) + "\r\n\r\n";
            fileResponse += content; 
                std::cout<<"\nSending response back to the client for GET and index path."<<std::endl;
            if (!sendAll(connAccept, fileResponse)) {
                std::cerr << "Failed to send complete response." << std::endl;
            }
        }else if (method == "GET" && path != "/") {
            // Remove the leading '/' from the path
            path.erase(0, 1);

            // Reject directory traversal attempts
            if (path.find("..") != std::string::npos) {
                std::string forbiddenResponse =
                    "HTTP/1.1 403 Forbidden\r\n"
                    "Content-Type: text/html\r\n"
                    "Content-Length: 48\r\n"
                    "Connection: close\r\n"
                    "\r\n"
                    "<html><body><h1>403 Forbidden</h1></body></html>";

                sendAll(connAccept, forbiddenResponse);

                return;
            }

            // Open the requested file
            std::ifstream file("public/" + path);

            if (!file.is_open()) {
                std::cout << "Failed to open file: " << path <<std::endl;

                std::string notFoundResponse =
                    "HTTP/1.1 404 Not Found\r\n"
                    "Content-Type: text/html\r\n"
                    "Content-Length: 48\r\n"
                    "Connection: close\r\n"
                    "\r\n"
                    "<html><body><h1>404 Not Found</h1></body></html>";

                sendAll(connAccept, notFoundResponse);

                return;
            }

            // Read the file contents
            std::string fileContent{
                std::istreambuf_iterator<char>(file),
                std::istreambuf_iterator<char>()
            };

            size_t contentSize = fileContent.size();

            std::string fileResponse =
                response +
                "Connection: close\r\n" +
                "Content-Length: " + std::to_string(contentSize) +
                "\r\n\r\n" +
                fileContent;

            std::cout << "Sending response for file: " << path << std::endl;

            //Check for partial send and handle it
            if (!sendAll(connAccept, fileResponse)) {
                std::cerr << "Failed to send complete response." << std::endl;
            } else {
                std::cout << "Response sent successfully." << std::endl;
            }
        }
        buffer[0] = '\0'; // Clear the buffer for the next

        std::cout << "END socket " << connAccept
          << " thread " << std::this_thread::get_id()
          << std::endl;
}
