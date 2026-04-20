# ThreadPool

A minimal C++ thread pool implementation using C++17 standard library primitives.

## Overview

A fixed-size pool of worker threads that accepts tasks via a thread-safe queue. Workers block when idle and wake when work is available. The pool drains all pending tasks before shutting down on destruction.

## Implementation

- `std::thread` for worker threads
- `std::mutex` and `std::condition_variable` for synchronization
- `std::queue<std::function<void()>>` for the task queue
- RAII-based lifetime — no manual cleanup required

## Usage

```cpp
ThreadPool pool(4); // spawn 4 worker threads

pool.submit([]() {
    std::cout << "hello from thread " << std::this_thread::get_id() << "\n";
});
// destructor joins all threads and drains remaining tasks
```

## Build

```bash
make
./threadpool
```

Requires g++ with C++17 support and pthreads (Linux/macOS).

## Notes

- Copy and move operations are explicitly deleted — a thread pool cannot be copied or moved
- Tasks are submitted as `std::function<void()>` lambdas
- Workers use condition variable wait with predicate to guard against spurious wakeups