#ifndef CUSTOM_ALLOCATOR_HPP
#define CUSTOM_ALLOCATOR_HPP

#include <memory_resource>
#include <memory>
#include <vector>
#include <exception>
#include <mutex>
#include <iostream>

class CustomMemoryResource: public std::pmr::memory_resource {

    struct MemoryBlock {
        size_t offset{0};
        size_t size{0};
    };

private:

    static constexpr std::size_t BUFFER_SIZE{1024};
    alignas(16) char _buffer[BUFFER_SIZE];
    std::vector<MemoryBlock> _used_blocks{{BUFFER_SIZE, 0}};
    std::mutex _mutex;

public:

    ~CustomMemoryResource() override = default;

public:

    std::size_t _get_buffer_size() const {
        return BUFFER_SIZE;
    }

private:

    void* do_allocate(std::size_t bytes, std::size_t alignment) override {
        std::cout << "TRY: Allocation: size: " << bytes << " bytes" << std::endl;
        std::lock_guard<std::mutex> lock(_mutex);
        std::size_t allocation_offset{0};
        std::size_t index{0};
        for (const MemoryBlock& used_block : _used_blocks) {
            void *tmp_ptr = _buffer + allocation_offset;
            std::size_t space = used_block.offset - allocation_offset;
            std::size_t remained_space{space};
            void *aligned_ptr = std::align(alignment, bytes, tmp_ptr, remained_space);
            if (aligned_ptr) {
                allocation_offset += space - remained_space;
                _used_blocks.emplace(
                    _used_blocks.begin() + index,
                    allocation_offset,
                    bytes
                );
                std::cout << "Allocation: offset " << allocation_offset << ", size " << bytes << " bytes" << std::endl;
                return _buffer + allocation_offset;
            }
            allocation_offset = used_block.offset + used_block.size;
            ++index;
        }
        throw std::bad_alloc();
    }

    void do_deallocate(void* ptr, std::size_t bytes, std::size_t alignment) override {
        std::cout << "TRY: Free: address " << ptr << ", size " << bytes << " bytes" << std::endl;
        std::lock_guard<std::mutex> lock(_mutex);
        std::size_t a{0};
        std::size_t b{_used_blocks.size() - 2};
        while (a <= b) {
            std::size_t k{(a + b) / 2};
            void* current_ptr = static_cast<void*>(_buffer + _used_blocks[k].offset);
            if (ptr == current_ptr) {
                if (_used_blocks[k].size != bytes) {
                    throw std::logic_error("An attempt to free an incorrectly sized block.");
                }
                std::cout << "Free: address " << ptr << ", offset: " << _used_blocks[k].offset << ", size " << bytes << " bytes" << std::endl;
                _used_blocks.erase(_used_blocks.begin() + k);
                return;
            }
            if (ptr < current_ptr) {
                b = k - 1;
            } else {
                a = k + 1;
            }
        }
        throw std::logic_error("An attempt to free an unallocated block.");
    }

    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
        return this == &other;
    }
};

#endif