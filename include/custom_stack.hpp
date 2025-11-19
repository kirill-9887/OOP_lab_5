#ifndef CUSTOM_STACK_HPP
#define CUSTOM_STACK_HPP

#include <concepts>
#include <memory_resource>
#include <memory>

template<typename StackType>
class CustomStackIterator {
private:
    StackType* _stack_ptr;
    size_t _current_index;

public:
    CustomStackIterator(StackType *stack_ptr, size_t index):
        _stack_ptr(stack_ptr), _current_index(index) {
    }

    ~CustomStackIterator() = default;

    typename StackType::item_type operator*() {
        if (_current_index >= (*_stack_ptr).size()) {
            throw std::out_of_range("Stack iterator is out of range");
        }
        return (*_stack_ptr)._data_ptr.get()[_current_index];
    }
    
    bool operator==(const CustomStackIterator<StackType>& other) const {
        return (other._current_index == _current_index) && (other._stack_ptr == _stack_ptr);
    }

    bool operator!=(const CustomStackIterator<StackType>& other) const {
        return !(*this == other);
    }

    CustomStackIterator<StackType>& operator++() {
        ++_current_index;
        return *this;
    }

    CustomStackIterator<StackType> operator++(int) {
        CustomStackIterator<StackType> tmp{*this};
        ++_current_index;
        return tmp;
    }
};

template <typename T, typename allocator_type>
requires std::is_default_constructible_v<T> && 
         std::is_same_v<allocator_type, std::pmr::polymorphic_allocator<T>>
class CustomStack {

    template<typename A>
    friend class CustomStackIterator;

private:
    struct PolymorphicDeleter {
        void operator()(T* ptr) const {}
    };
    
    allocator_type _polymorphic_allocator;
    std::unique_ptr<T, PolymorphicDeleter> _data_ptr;
    std::size_t _capacity;
    std::size_t _size{0};

public:
    using item_type = T;

    CustomStack(std::size_t size, allocator_type alloc = {}) :
        _polymorphic_allocator(alloc), _capacity(size), _size(0) {
        T* raw_ptr = _polymorphic_allocator.allocate(size);
        for (size_t element_index = 0; element_index < size; ++element_index) {
            _polymorphic_allocator.construct(raw_ptr + element_index);
        }
        _data_ptr = std::unique_ptr<T, PolymorphicDeleter>(raw_ptr, PolymorphicDeleter{});
    }

    ~CustomStack() {
        if constexpr (std::is_destructible_v<T>) {
            for (size_t element_index = 0; element_index < _capacity; ++element_index) {
                std::allocator_traits<allocator_type>::destroy(_polymorphic_allocator, 
                    _data_ptr.get() + element_index);
                }
            }
        _polymorphic_allocator.deallocate(_data_ptr.get(), _capacity);
    }

    std::size_t size() const {
        return _size;
    }

    bool empty() const {
        return _size == 0;
    }

    void push(const T &item) {
        if (_size == _capacity) {
            throw std::out_of_range("Stack overflow");
        }
        _data_ptr.get()[_size] = item;
        ++_size;
    }

    void pop() {
        --_size;
    }

    T& top() {
        if (empty()) {
            throw std::out_of_range("Stack is empty");
        }
        return _data_ptr.get()[_size - 1];
    }
    
    const T& top() const {
        if (empty()) {
            throw std::out_of_range("Stack is empty");
        }
        return _data_ptr.get()[_size - 1];
    }
    
    CustomStackIterator<CustomStack<T, allocator_type>> begin() {
        return CustomStackIterator<CustomStack<T, allocator_type>>(this, 0);
    }
    
    CustomStackIterator<CustomStack<T, allocator_type>> end() {
        return CustomStackIterator<CustomStack<T, allocator_type>>(this, _size);
    }

    CustomStackIterator<const CustomStack<T, allocator_type>> begin() const {
        return CustomStackIterator<const CustomStack<T, allocator_type>>(this, 0);
    }
    
    CustomStackIterator<const CustomStack<T, allocator_type>> end() const {
        return CustomStackIterator<const CustomStack<T, allocator_type>>(this, _size);
    }
};

#endif