# C++ HTTP Server

A lightweight HTTP server built from scratch in C++ using POSIX sockets. This project explores low-level networking, HTTP request handling, and concurrent client processing with a custom thread pool.

## Features

- TCP socket creation, binding, and listening on port `8080`
- Basic HTTP `GET` request handling
- Static file serving
- Custom thread pool for concurrent client connections
- Shared task queue protected by `std::mutex`
- Worker coordination using `std::condition_variable`
- C++17 implementation with POSIX socket APIs

## Tech Stack

- **Language:** C++17
- **Networking:** POSIX sockets, TCP/IP
- **Concurrency:** `std::thread`, `std::mutex`, `std::condition_variable`
- **Compiler:** Clang or GCC
- **Platform:** macOS and Linux

## Project Structure

```text
httpServer/
├── main.cpp
├── serverheader.h
├── ThreadPool.h
├── index.html
├── Makefile
└── README.md
```

*Adjust filenames to match the exact capitalization in your repository.*

## Build

### Requirements

- A C++17-compatible compiler
- `make`
- macOS or Linux

Compile the project:

```bash
make
```

If you do not have a Makefile yet, compile directly:

```bash
clang++ -std=c++17 -Wall -Wextra -pthread main.cpp -o server
```

## Run the Server

Start the server:

```bash
./server
```

The server listens on port `8080`.

Open this URL in your browser:

```text
http://localhost:8080/
```

Or test it with `curl`:

```bash
curl -i http://localhost:8080/
```

## Test Concurrent Requests

With the server running, open another terminal and send multiple requests concurrently:

```bash
for i in {1..10}; do
    curl -s -o /dev/null http://localhost:8080/ &
done
wait
```

The server uses a fixed-size worker pool to process accepted client connections. When all workers are busy, additional accepted connections wait in the task queue until a worker becomes available.

## How the Thread Pool Works

1. The main thread accepts a client connection.
2. The client socket descriptor is added to a shared task queue.
3. Idle workers wait on a condition variable until work becomes available.
4. A worker locks the queue mutex, retrieves one task, and removes it from the queue.
5. The worker releases the mutex before processing the request, allowing other workers to retrieve tasks concurrently.
6. During shutdown, the thread pool signals its workers and joins them.

The mutex protects access to the shared queue, while the condition variable prevents idle workers from continuously polling for new tasks.

## Current Limitations

This is a learning project and is not intended for production use yet.

Areas for improvement include:

- More robust HTTP request parsing
- Handling partial socket reads and writes
- Preventing directory traversal through requested file paths
- Accurate HTTP response headers and `Content-Length`
- Improved socket error handling and cleanup
- Thread-safe logging
- Automated tests for valid requests, missing files, and concurrent connections
- More explicit handling of shutdown and queued connections

## Roadmap

- [ ] Resolve compiler warnings
- [ ] Improve HTTP request parsing and response handling
- [ ] Add automated tests
- [ ] Improve logging and error handling
- [ ] Review thread-pool shutdown and socket ownership
- [ ] Deploy to AWS EC2
- [ ] Document a public demo endpoint after deployment

## Learning Objectives

This project demonstrates practical work with:

- TCP networking and socket programming
- HTTP request/response handling
- Multithreading and concurrent execution
- Mutexes, condition variables, and critical sections
- Producer-consumer queues
- Thread-pool architecture
- Linux server deployment

## Project Status

**In development.** Basic HTTP requests and concurrent client processing have been tested locally. AWS deployment is planned but has not yet been completed.
