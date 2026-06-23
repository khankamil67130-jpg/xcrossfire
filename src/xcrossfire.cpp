#include "xcrossfire.hpp"
#include <immintrin.h>
#include <atomic>
#include <thread>
#include <chrono>
#include <type_traits>

std::atomic<int64_t> global_target_index{-1};
std::atomic<bool> global_target_found{false};
std::atomic<bool> system_shutdown{false};
std::atomic<bool> start_signal{false};
std::atomic<int> completed_channels{0};

const void* global_ptr = nullptr;
size_t global_size = 0;

struct UniversalTarget {
    int i_val;
    float f_val;
} global_target;

enum DataType { TYPE_INT, TYPE_FLOAT } active_mode;

// AVX2 Blocks for Integer
inline void simd_scan_block(const int* ptr, size_t i, int target, __m256i& v_target) {
    _mm_prefetch((const char*)(ptr + i + 128), _MM_HINT_T0);
    __m256i v_data0 = _mm256_load_si256((const __m256i*)(ptr + i));
    __m256i v_data1 = _mm256_load_si256((const __m256i*)(ptr + i + 8));
    __m256i v_data2 = _mm256_load_si256((const __m256i*)(ptr + i + 16));
    __m256i v_data3 = _mm256_load_si256((const __m256i*)(ptr + i + 24));
    int mask0 = _mm256_movemask_epi8(_mm256_cmpeq_epi32(v_data0, v_target));
    int mask1 = _mm256_movemask_epi8(_mm256_cmpeq_epi32(v_data1, v_target));
    int mask2 = _mm256_movemask_epi8(_mm256_cmpeq_epi32(v_data2, v_target));
    int mask3 = _mm256_movemask_epi8(_mm256_cmpeq_epi32(v_data3, v_target));
    if (mask0 != 0) { global_target_index = i + __builtin_ctz(mask0) / 4; global_target_found = true; }
    else if (mask1 != 0) { global_target_index = i + 8 + __builtin_ctz(mask1) / 4; global_target_found = true; }
    else if (mask2 != 0) { global_target_index = i + 16 + __builtin_ctz(mask2) / 4; global_target_found = true; }
    else if (mask3 != 0) { global_target_index = i + 24 + __builtin_ctz(mask3) / 4; global_target_found = true; }
}

// AVX2 Blocks for Float
inline void simd_scan_block(const float* ptr, size_t i, float target, __m256& v_target) {
    _mm_prefetch((const char*)(ptr + i + 128), _MM_HINT_T0);
    __m256 v_data0 = _mm256_load_ps(ptr + i);
    __m256 v_data1 = _mm256_load_ps(ptr + i + 8);
    __m256 v_data2 = _mm256_load_ps(ptr + i + 16);
    __m256 v_data3 = _mm256_load_ps(ptr + i + 24);
    int mask0 = _mm256_movemask_ps(_mm256_cmp_ps(v_data0, v_target, _CMP_EQ_OQ));
    int mask1 = _mm256_movemask_ps(_mm256_cmp_ps(v_data1, v_target, _CMP_EQ_OQ));
    int mask2 = _mm256_movemask_ps(_mm256_cmp_ps(v_data2, v_target, _CMP_EQ_OQ));
    int mask3 = _mm256_movemask_ps(_mm256_cmp_ps(v_data3, v_target, _CMP_EQ_OQ));
    if (mask0 != 0) { global_target_index = i + __builtin_ctz(mask0); global_target_found = true; }
    else if (mask1 != 0) { global_target_index = i + 8 + __builtin_ctz(mask1); global_target_found = true; }
    else if (mask2 != 0) { global_target_index = i + 16 + __builtin_ctz(mask2); global_target_found = true; }
    else if (mask3 != 0) { global_target_index = i + 24 + __builtin_ctz(mask3); global_target_found = true; }
}

void persistent_forward_worker() {
    while (!system_shutdown.load(std::memory_order_relaxed)) {
        while (!start_signal.load(std::memory_order_relaxed)) {
            if (system_shutdown.load(std::memory_order_relaxed)) return;
            _mm_pause();
        }
        if (active_mode == TYPE_INT) {
            const int* ptr = reinterpret_cast<const int*>(global_ptr);
            size_t size = global_size;
            int target = global_target.i_val;
            __m256i v_target = _mm256_set1_epi32(target);
            size_t i = 0;
            for (; i + 32 <= size; i += 32) { if (global_target_found.load(std::memory_order_relaxed)) break; simd_scan_block(ptr, i, target, v_target); }
            if (!global_target_found.load(std::memory_order_relaxed)) { for (; i < size; ++i) if (ptr[i] == target) { global_target_index = i; global_target_found = true; break; } }
        } 
        else if (active_mode == TYPE_FLOAT) {
            const float* ptr = reinterpret_cast<const float*>(global_ptr);
            size_t size = global_size;
            float target = global_target.f_val;
            __m256 v_target = _mm256_set1_ps(target);
            size_t i = 0;
            for (; i + 32 <= size; i += 32) { if (global_target_found.load(std::memory_order_relaxed)) break; simd_scan_block(ptr, i, target, v_target); }
            if (!global_target_found.load(std::memory_order_relaxed)) { for (; i < size; ++i) if (ptr[i] == target) { global_target_index = i; global_target_found = true; break; } }
        }
        completed_channels.fetch_add(1, std::memory_order_release);
        while (start_signal.load(std::memory_order_relaxed)) { _mm_pause(); }
    }
}

void persistent_backward_worker() {
    while (!system_shutdown.load(std::memory_order_relaxed)) {
        while (!start_signal.load(std::memory_order_relaxed)) {
            if (system_shutdown.load(std::memory_order_relaxed)) return;
            _mm_pause();
        }
        if (active_mode == TYPE_INT) {
            const int* ptr = reinterpret_cast<const int*>(global_ptr);
            size_t size = global_size;
            int target = global_target.i_val;
            __m256i v_target = _mm256_set1_epi32(target);
            int64_t i = static_cast<int64_t>(size) - 32;
            for (; i >= 0; i -= 32) { if (global_target_found.load(std::memory_order_relaxed)) break; simd_scan_block(ptr, i, target, v_target); }
            if (!global_target_found.load(std::memory_order_relaxed)) { i += 31; for (; i >= 0; --i) if (ptr[i] == target) { global_target_index = i; global_target_found = true; break; } }
        } 
        else if (active_mode == TYPE_FLOAT) {
            const float* ptr = reinterpret_cast<const float*>(global_ptr);
            size_t size = global_size;
            float target = global_target.f_val;
            __m256 v_target = _mm256_set1_ps(target);
            int64_t i = static_cast<int64_t>(size) - 32;
            for (; i >= 0; i -= 32) { if (global_target_found.load(std::memory_order_relaxed)) break; simd_scan_block(ptr, i, target, v_target); }
            if (!global_target_found.load(std::memory_order_relaxed)) { i += 31; for (; i >= 0; --i) if (ptr[i] == target) { global_target_index = i; global_target_found = true; break; } }
        }
        completed_channels.fetch_add(1, std::memory_order_release);
        while (start_signal.load(std::memory_order_relaxed)) { _mm_pause(); }
    }
}

std::thread forward_thread;
std::thread backward_thread;

extern "C" {
    void xcf_boot_engine() {
        system_shutdown = false; start_signal = false; completed_channels = 0;
        forward_thread = std::thread(persistent_forward_worker);
        backward_thread = std::thread(persistent_backward_worker);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    void xcf_shutdown_engine() {
        system_shutdown = true;
        if (forward_thread.joinable()) forward_thread.join();
        if (backward_thread.joinable()) backward_thread.join();
    }

    int64_t xcf_search_int(const int* ptr, size_t size, int target) {
        global_target_index = -1; global_target_found = false; completed_channels.store(0, std::memory_order_relaxed);
        global_ptr = reinterpret_cast<const void*>(ptr); global_size = size;
        active_mode = TYPE_INT; global_target.i_val = target;
        start_signal.store(true, std::memory_order_release);
        while (completed_channels.load(std::memory_order_acquire) < 2) { _mm_pause(); }
        start_signal.store(false, std::memory_order_release);
        return global_target_index.load(std::memory_order_relaxed);
    }

    int64_t xcf_search_float(const float* ptr, size_t size, float target) {
        global_target_index = -1; global_target_found = false; completed_channels.store(0, std::memory_order_relaxed);
        global_ptr = reinterpret_cast<const void*>(ptr); global_size = size;
        active_mode = TYPE_FLOAT; global_target.f_val = target;
        start_signal.store(true, std::memory_order_release);
        while (completed_channels.load(std::memory_order_acquire) < 2) { _mm_pause(); }
        start_signal.store(false, std::memory_order_release);
        return global_target_index.load(std::memory_order_relaxed);
    }
}
