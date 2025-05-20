#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <cstdint>

// Forward declaration for Object class
class Object;

// High Performance Thread Pool implementation
class HighPerformanceThreadPool {
public:
    // Constructor initializes the thread pool with maximum threads
    HighPerformanceThreadPool();
    
    // Destructor ensures all threads are properly joined
    ~HighPerformanceThreadPool();
    
    // Adds a new task to the pool
    template<class F>
    void enqueue(F&& f) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            tasks.emplace(std::forward<F>(f));
        }
        condition.notify_one();
    }
    
    // Wait until all tasks are completed
    void wait_idle();
    
    // Increment active task counter
    void increment_active();
    
    // Decrement active task counter
    void decrement_active();
    
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    std::condition_variable condition_idle;
    bool stop;
    int active_tasks = 0;
};

// Z-Order utility functions
struct ZOrderedBody {
    Object* body;
    uint64_t zcode;
    
    bool operator<(const ZOrderedBody& other) const {
        return zcode < other.zcode;
    }
};

// Interleave bits to create Z-order (Morton) code
uint64_t interleave_bits(uint32_t x, uint32_t y);

// Compute Z-order code for spatial position
uint64_t compute_zorder(float x, float y, const std::vector<float>& min_bounds, const std::vector<float>& max_bounds);

#endif // THREADPOOL_H