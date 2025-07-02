#ifndef DAB_NODE_ARENA_H
#define DAB_NODE_ARENA_H

#include <cstddef>
#include <vector>
#include <new>
#include <utility>
#include <memory>

// 轻量线程本地对象池，实现固定 2 KiB 块的顺序分配，
// reset() 时一次性回收全部内存，O(1) 释放。
class Arena {
public:
    static constexpr std::size_t BLOCK_SIZE = 2 * 1024; // 2 KiB
    static constexpr std::size_t SLAB_CAPACITY = BLOCK_SIZE * 1024; // 2MiB per slab (容纳 1024 节点)

    Arena()  { add_slab(); }

    void *alloc(std::size_t sz) {
        constexpr std::size_t align = alignof(std::max_align_t);

        if (sz > BLOCK_SIZE) {
            // 极端情况，直接用全局 new
            return ::operator new(sz);
        }

        // 对齐当前偏移
        std::size_t aligned_offset = (_offset + (align - 1)) & ~(align - 1);

        if (aligned_offset + BLOCK_SIZE > SLAB_CAPACITY) {
            add_slab();
            aligned_offset = 0;
        }

        char *ptr = _current_slab + aligned_offset;
        _offset = aligned_offset + BLOCK_SIZE;
        return static_cast<void *>(ptr);
    }

    void reset() noexcept {
        // 释放所有 slab
        _slabs.clear();
        _current_slab = nullptr;
        _offset = 0;
        // 重新分配首 slab 以便继续使用
        add_slab();
    }

private:
    void add_slab() {
        _slabs.emplace_back(new char[SLAB_CAPACITY]);
        _current_slab = _slabs.back().get();
        _offset = 0;
    }

    std::vector<std::unique_ptr<char[]>> _slabs; // 所有 slab
    char* _current_slab = nullptr;               // 当前 slab 原始指针
    std::size_t _offset = 0;               // 当前 slab 已用偏移
};

// 线程本地 Arena 实例，仅声明；在 NodeArena.cpp 定义一次
extern thread_local Arena NODE_TT_ARENA;

// 返回线程本地 Arena
inline Arena &get_node_arena() noexcept {
    return NODE_TT_ARENA;
}

#endif // DAB_NODE_ARENA_H 