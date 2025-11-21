#ifndef CUSTOM_STACK_HPP
#define CUSTOM_STACK_HPP

#include <concepts>
#include <memory_resource>
#include <memory>

template<typename StackType>
class CustomStackIterator
{
private:
    
    const StackType* _stack_ptr;
    std::size_t _current_index;

public:

    CustomStackIterator(const StackType *stack_ptr, std::size_t index):
        _stack_ptr(stack_ptr), _current_index(index) {
    }
    CustomStackIterator(const CustomStackIterator& other) = default;
    CustomStackIterator(CustomStackIterator&& other) noexcept = default;
    CustomStackIterator& operator=(const CustomStackIterator& other) = default;
    CustomStackIterator& operator=(CustomStackIterator&& other) noexcept = default;
    virtual ~CustomStackIterator() noexcept = default;

public:

    typename StackType::item_type operator*() const {
        if (_current_index >= (*_stack_ptr).size()) {
            throw std::out_of_range("Stack iterator is out of range");
        }
        return (*_stack_ptr)._data_ptr.get()[_current_index];
    }
    
    bool operator==(const CustomStackIterator<StackType>& other) const {
        return (_stack_ptr == other._stack_ptr) && (_current_index == other._current_index);
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
class CustomStack 
{
    template<typename A>
    friend class CustomStackIterator;

private:

    void free_data() {
        if constexpr (std::is_destructible_v<T>) {
            for (std::size_t element_index = 0; element_index < _capacity; ++element_index) {
                std::allocator_traits<allocator_type>::destroy(
                    _polymorphic_allocator, 
                    _data_ptr.get() + element_index
                );
            }
        }
        if (_data_ptr) {
            _polymorphic_allocator.deallocate(_data_ptr.get(), _capacity);
        }
    }

    struct PolymorphicDeleter {
        void operator()(T* ptr) const {
            // The memory is released through the allocator
        }
    };
    
    allocator_type _polymorphic_allocator;
    std::unique_ptr<T, PolymorphicDeleter> _data_ptr;
    std::size_t _capacity;
    std::size_t _size{0};
    std::size_t INITIAL_CAPACITY{1};
    int EXTENSION_COEF{2};

public:

    using item_type = T;

    CustomStack(std::size_t capacity, allocator_type alloc = {}) :
        _polymorphic_allocator(alloc),
        _capacity(capacity),
        _size(0)
    {
        T* raw_ptr = _polymorphic_allocator.allocate(capacity);
        for (std::size_t element_index = 0; element_index < capacity; ++element_index) {
            _polymorphic_allocator.construct(raw_ptr + element_index);
        }
        _data_ptr = std::unique_ptr<T, PolymorphicDeleter>(raw_ptr, PolymorphicDeleter{});
    }

    CustomStack(allocator_type alloc = {}) :
        CustomStack::CustomStack(INITIAL_CAPACITY, alloc) {
    }

    CustomStack(const CustomStack<T, allocator_type>& other, allocator_type alloc = {}) :
        _polymorphic_allocator(alloc),
        _capacity(other._capacity),
        _size(other._size)
    {
        T* raw_ptr = _polymorphic_allocator.allocate(other._capacity);
        std::uninitialized_copy(other._data_ptr.get(), other._data_ptr.get() + other._capacity, raw_ptr);
        _data_ptr = std::unique_ptr<T, PolymorphicDeleter>(raw_ptr, PolymorphicDeleter{});
    }

    CustomStack(CustomStack<T, allocator_type>&& other) noexcept :
        _polymorphic_allocator(std::move(other._polymorphic_allocator)),
        _capacity(other._capacity),
        _size(other._size),
        _data_ptr(std::move(other._data_ptr))
    {
        other._capacity = 0;
        other._size = 0;
    }

    CustomStack<T, allocator_type>& operator=(const CustomStack<T, allocator_type>& other) {
        if (this != &other) {
            CustomStack<T, allocator_type> copy(other, other._polymorphic_allocator);
            std::swap(_capacity, copy._capacity);
            std::swap(_size, copy._size);
            std::swap(_data_ptr, copy._data_ptr);
        }
        return *this;
    }

    CustomStack<T, allocator_type>& operator=(CustomStack<T, allocator_type>&& other) noexcept {
        if (this != &other) {
            free_data();
            _capacity = other._capacity;
            _size = other._size;
            T* raw_ptr = _polymorphic_allocator.allocate(other._capacity);
            std::uninitialized_move(other._data_ptr.get(), other._data_ptr.get() + other._capacity, raw_ptr);
            _data_ptr = std::unique_ptr<T, PolymorphicDeleter>(raw_ptr, PolymorphicDeleter{});
            other._size = 0;
        }
        return *this;
    }

    virtual ~CustomStack() {
        free_data();
    }

private:

    void __extend_capacity() {
        std::size_t new_capacity{_capacity * EXTENSION_COEF};
        T* raw_ptr = _polymorphic_allocator.allocate(new_capacity);
        try {
            for (std::size_t element_index = _size; element_index < new_capacity; ++element_index) {
                _polymorphic_allocator.construct(raw_ptr + element_index);
            }
        } catch (...) {
            _polymorphic_allocator.deallocate(raw_ptr, new_capacity);
            throw;
        }
        std::uninitialized_move(_data_ptr.get(), _data_ptr.get() + _size, raw_ptr);
        free_data();
        _data_ptr = std::unique_ptr<T, PolymorphicDeleter>(raw_ptr, PolymorphicDeleter{});
        _capacity = new_capacity;
    }

public:

    std::size_t size() const {
        return _size;
    }

    bool empty() const {
        return _size == 0;
    }

    void push(const T& item) {
        if (_size == _capacity) {
            __extend_capacity();
        }
        _data_ptr.get()[_size] = item;
        ++_size;
    }

    void push(T&& item) {
        if (_size == _capacity) {
            __extend_capacity();
        }
        _data_ptr.get()[_size] = std::move(item);
        ++_size;
    }

    void pop() {
        if (empty()) {
            throw std::out_of_range("Stack is empty");
        }
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