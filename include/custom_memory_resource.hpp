#ifndef CUSTOM_ALLOCATOR_HPP
#define CUSTOM_ALLOCATOR_HPP

#include <memory_resource>
#include <memory>
#include <vector>
#include <exception>
#include <mutex>
#include <iostream>

// TODO: bytes == 0

class CustomMemoryResource: public std::pmr::memory_resource {

    struct MemoryBlock {
        size_t offset{0};
        size_t size{0};
    };
    
    static constexpr size_t BUFFER_SIZE{1024};
    alignas(16) char buffer[BUFFER_SIZE];
    std::vector<MemoryBlock> used_blocks{{BUFFER_SIZE, 0}};
    std::mutex _mutex;

public:
    CustomMemoryResource() = default;
    ~CustomMemoryResource() = default;

    void* do_allocate(size_t bytes, size_t alignment) override {
        std::lock_guard<std::mutex> lock(_mutex);
        size_t allocation_offset{0};
        size_t index{0};
        for (const MemoryBlock& used_block : used_blocks) {
            void *tmp_ptr = buffer + allocation_offset;
            size_t space = used_block.offset - allocation_offset;
            size_t remained_space{space};
            void *aligned_ptr = std::align(alignment, bytes, tmp_ptr, remained_space);
            if (aligned_ptr) {
                allocation_offset += space - remained_space;
                used_blocks.emplace(
                    used_blocks.begin() + index,
                    allocation_offset,
                    bytes
                );
                std::cout << "Выделение: смещение " << allocation_offset << ", размер " << bytes << " байт" << std::endl;
                return buffer + allocation_offset;
            }
            allocation_offset = used_block.offset + used_block.size;
            ++index;
        }
        throw std::bad_alloc();
    }

    void do_deallocate(void* ptr, size_t bytes, size_t alignment) override {
        std::lock_guard<std::mutex> lock(_mutex);
        std::cout << "Освобождение: адрес " << ptr << ", размер " << bytes << " байт" << std::endl;
        size_t a{0};
        size_t b{used_blocks.size() - 2};
        while (a <= b) {
            size_t k{(a + b) / 2};
            void* current_ptr = static_cast<void*>(buffer + used_blocks[k].offset);
            if (ptr == current_ptr) {
                used_blocks.erase(used_blocks.begin() + k);
                return;
            }
            if (ptr < current_ptr) {
                b = k - 1;
            } else {
                a = k + 1;
            }
        }
        throw std::logic_error("Попытка освобождения не выделенного блока");
    }

    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
        return this == &other;
    }
};

#endif