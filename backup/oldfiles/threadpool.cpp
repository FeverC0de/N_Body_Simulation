#include "object.h"
#include "threadpool.h"
#include <algorithm>  // For std::max

#ifdef _WIN32
#include <Windows.h>
#endif

HighPerformanceThreadPool::HighPerformanceThreadPool() : stop(false) {
    // Use maximum available hardware threads
    size_t num_threads = std::max<size_t>(2, std::thread::hardware_concurrency());
    std::cout << "MAXIMIZING PERFORMANCE: Using " << num_threads << " threads in the pool" << std::endl;
    
    // Set process priority if on Windows
    #ifdef _WIN32
    try {
        SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
        std::cout << "Process priority set to HIGH" << std::endl;
    } catch(...) {
        std::cerr << "Failed to set process priority" << std::endl;
    }
    #endif
    
    // Create worker threads
    for (size_t i = 0; i < num_threads; ++i) {
        workers.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(queue_mutex);
                    condition.wait(lock, [this] { 
                        return stop || !tasks.empty(); 
                    });
                    
                    if (stop && tasks.empty()) return;
                    task = std::move(tasks.front());
                    tasks.pop();
                }
                
                // Execute the task
                task();
            }
        });
    }
}

HighPerformanceThreadPool::~HighPerformanceThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for (std::thread &worker : workers) {
        worker.join();
    }
}

void HighPerformanceThreadPool::wait_idle() {
    std::unique_lock<std::mutex> lock(queue_mutex);
    condition_idle.wait(lock, [this] { 
        return tasks.empty() && active_tasks == 0; 
    });
}

void HighPerformanceThreadPool::increment_active() {
    std::unique_lock<std::mutex> lock(queue_mutex);
    active_tasks++;
}

void HighPerformanceThreadPool::decrement_active() {
    std::unique_lock<std::mutex> lock(queue_mutex);
    active_tasks--;
    if (tasks.empty() && active_tasks == 0) {
        condition_idle.notify_all();
    }
}

// Interleave bits to create Z-order (Morton) code
uint64_t interleave_bits(uint32_t x, uint32_t y) {
    static const uint64_t B[] = {0x5555555555555555ULL, 0x3333333333333333ULL, 0x0F0F0F0F0F0F0F0FULL, 
                              0x00FF00FF00FF00FFULL, 0x0000FFFF0000FFFFULL};
    static const unsigned int S[] = {1, 2, 4, 8, 16};
    
    uint64_t xx = x;
    uint64_t yy = y;
    
    xx = (xx | (xx << S[4])) & B[4];
    xx = (xx | (xx << S[3])) & B[3];
    xx = (xx | (xx << S[2])) & B[2];
    xx = (xx | (xx << S[1])) & B[1];
    xx = (xx | (xx << S[0])) & B[0];
    
    yy = (yy | (yy << S[4])) & B[4];
    yy = (yy | (yy << S[3])) & B[3];
    yy = (yy | (yy << S[2])) & B[2];
    yy = (yy | (yy << S[1])) & B[1];
    yy = (yy | (yy << S[0])) & B[0];
    
    return xx | (yy << 1);
}

// Compute Z-order code for spatial position
uint64_t compute_zorder(float x, float y, const std::vector<float>& min_bounds, const std::vector<float>& max_bounds) {
    // Normalize coordinates to 0-1 range
    float norm_x = (x - min_bounds[0]) / (max_bounds[0] - min_bounds[0]);
    float norm_y = (y - min_bounds[1]) / (max_bounds[1] - min_bounds[1]);
    
    // Scale to 32-bit integer range
    uint32_t ix = static_cast<uint32_t>(norm_x * UINT32_MAX);
    uint32_t iy = static_cast<uint32_t>(norm_y * UINT32_MAX);
    
    // Interleave bits to create Z-order code
    return interleave_bits(ix, iy);
}