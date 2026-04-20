#include <iostream>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>
#include <queue>
#include <functional>

class ThreadPool {
    public:
        explicit ThreadPool(int num_threads) {
            for (int i = 0; i < num_threads; ++i) {
                thread_list.emplace_back(&ThreadPool::worker_function, this);
            }
        }

        ~ThreadPool() {
            // the stop bool is actually only ever evaluated by workers when they have the lock (such as when wait finishes)
            std::unique_lock<std::mutex> stop_lock{queue_mut};
            stop_bool = true;
            stop_lock.unlock();
            cv.notify_all(); // blocked workers will wake and see stop_bool, terminate; running workers will reacquire the lock, verify queue is empty, see stop_bool, and terminate
            for (std::thread& t : thread_list) {
                t.join();
            }
        }

        ThreadPool(const ThreadPool&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;
        ThreadPool(ThreadPool&&) = delete;
        ThreadPool& operator=(ThreadPool&&) = delete;
        
        void submit(std::function<void()> task) {
            std::unique_lock<std::mutex> submit_lock{queue_mut};
            if (stop_bool) { // in case another thread tries to submit during deconstruction
                return;
            }
            task_queue.push(std::move(task));
            submit_lock.unlock();
            cv.notify_one();
        }

    private:
        void worker_function() {
            std::unique_lock<std::mutex> task_lk(queue_mut);
            cv.wait(task_lk, [&]{ return !task_queue.empty() || stop_bool; });
            while(!stop_bool || !task_queue.empty()) {
                while(!task_queue.empty()) {
                    std::function<void()> fun = task_queue.front();
                    task_queue.pop();
                    task_lk.unlock();
                    fun();
                    task_lk.lock();
                }
                if (stop_bool) { //only ever evaluated if the queue is empty AND thread has the lock
                    return;
                }
                cv.wait(task_lk, [&]{ return !task_queue.empty() || stop_bool; });
            }
        }

        std::condition_variable cv;
        std::mutex queue_mut;
        std::vector<std::thread> thread_list;
        std::queue<std::function<void()>> task_queue;
        bool stop_bool = false;
};

int main () {
    ThreadPool tp{4};
    for (int i = 1; i < 21; ++i) {
        tp.submit([=]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::cout << "Task " << i << " ran on Thread " << std::this_thread::get_id() << "\n";
        });
    }
}