#include <gtest/gtest.h>
#include "../include/custom_memory_resource.hpp"
#include "../include/custom_stack.hpp"

TEST(AllocatorTest, MemoryTest) {
    CustomMemoryResource custom_memory_resource;
    std::pmr::polymorphic_allocator<char> polymorphic_allocator(&custom_memory_resource);
    char *buffer = polymorphic_allocator.allocate(1024);
    EXPECT_THROW(polymorphic_allocator.allocate(1), std::bad_alloc);
    EXPECT_DEATH(polymorphic_allocator.deallocate(buffer + 1, 1024), ".*");
    EXPECT_THROW(polymorphic_allocator.allocate(256),std::bad_alloc);
    EXPECT_NO_THROW(polymorphic_allocator.deallocate(buffer, 1024));
    EXPECT_NO_THROW(polymorphic_allocator.deallocate(polymorphic_allocator.allocate(256), 256));
}

TEST(StackTest, MemoryTest) {
    CustomMemoryResource custom_memory_resource;
    std::pmr::polymorphic_allocator<int> polymorphic_allocator(&custom_memory_resource);
    EXPECT_NO_THROW((CustomStack<int, std::pmr::polymorphic_allocator<int>>(0, polymorphic_allocator)));
    EXPECT_NO_THROW((CustomStack<int, std::pmr::polymorphic_allocator<int>>(10, polymorphic_allocator)));
    EXPECT_NO_THROW((CustomStack<int, std::pmr::polymorphic_allocator<int>>(custom_memory_resource._get_buffer_size() / sizeof(int), polymorphic_allocator)));
    EXPECT_ANY_THROW((CustomStack<int, std::pmr::polymorphic_allocator<int>>(custom_memory_resource._get_buffer_size() / sizeof(int) + 1, polymorphic_allocator)));
}

TEST(StackTest, IntItemType) {
    CustomMemoryResource custom_memory_resource;
    std::pmr::polymorphic_allocator<int> polymorphic_allocator(&custom_memory_resource);
    std::size_t capacity{20};
    CustomStack<int, std::pmr::polymorphic_allocator<int>> stack(capacity, polymorphic_allocator);
    
    EXPECT_TRUE(stack.empty());
    int a{1};
    stack.push(a);
    EXPECT_FALSE(stack.empty());
    EXPECT_EQ(stack.size(), 1);
    EXPECT_EQ(stack.top(), a);
    int b{2};
    stack.push(b);
    EXPECT_EQ(stack.top(), b);
    EXPECT_EQ(stack.size(), 2);
    stack.pop();
    EXPECT_EQ(stack.top(), a);
    EXPECT_EQ(stack.size(), 1);
    EXPECT_FALSE(stack.empty());
    stack.pop();
    EXPECT_TRUE(stack.empty());
    EXPECT_EQ(stack.size(), 0);
    
    EXPECT_ANY_THROW(stack.pop());
    for (std::size_t i{0}; i < capacity; ++i) {
        EXPECT_NO_THROW(stack.push(static_cast<int>(i)));
    }
    EXPECT_ANY_THROW(stack.push(10));
}

TEST(StackTest, StructureItemType) {
    struct TestStructure {
        int first_value;
        int second_value;

        bool operator==(const TestStructure& other) const {
            return first_value == other.first_value && second_value == other.second_value;
        }
    };

    CustomMemoryResource custom_memory_resource;
    std::pmr::polymorphic_allocator<TestStructure> polymorphic_allocator(&custom_memory_resource);
    std::size_t capacity{20};
    CustomStack<TestStructure, std::pmr::polymorphic_allocator<TestStructure>> stack(capacity, polymorphic_allocator);

    EXPECT_TRUE(stack.empty());
    TestStructure a{1, 2};
    stack.push(a);
    EXPECT_FALSE(stack.empty());
    EXPECT_EQ(stack.size(), 1);
    EXPECT_EQ(stack.top(), a);
    TestStructure b{3, 4};
    stack.push(b);
    EXPECT_EQ(stack.top(), b);
    EXPECT_EQ(stack.size(), 2);
    stack.pop();
    EXPECT_EQ(stack.top(), a);
    EXPECT_EQ(stack.size(), 1);
    EXPECT_FALSE(stack.empty());
    stack.pop();
    EXPECT_TRUE(stack.empty());
    EXPECT_EQ(stack.size(), 0);

    EXPECT_ANY_THROW(stack.pop());
    for (std::size_t i{0}; i < capacity; ++i) {
        EXPECT_NO_THROW(stack.push({static_cast<int>(i), static_cast<int>(i)}));
    }
    EXPECT_ANY_THROW(stack.push({10, 10}));
}

TEST(StackTest, CopyConstructor) {
    CustomMemoryResource custom_memory_resource;
    std::pmr::polymorphic_allocator<int> polymorphic_allocator(&custom_memory_resource);
    std::size_t capacity{20};
    CustomStack<int, std::pmr::polymorphic_allocator<int>> stack(capacity, polymorphic_allocator);
    stack.push(1);
    stack.push(2);
    CustomStack<int, std::pmr::polymorphic_allocator<int>> copy(stack, polymorphic_allocator);
    copy.push(3);
    EXPECT_EQ(stack.top(), 2);
    EXPECT_EQ(copy.top(), 3);
}

TEST(StackTest, MoveConstructor) {
    CustomMemoryResource custom_memory_resource;
    std::pmr::polymorphic_allocator<int> polymorphic_allocator(&custom_memory_resource);
    std::size_t capacity{20};
    CustomStack<int, std::pmr::polymorphic_allocator<int>> stack(capacity, polymorphic_allocator);
    stack.push(1);
    stack.push(2);
    CustomStack<int, std::pmr::polymorphic_allocator<int>> move(std::move(stack));
    EXPECT_TRUE(stack.empty());
    EXPECT_EQ(move.top(), 2);
    EXPECT_EQ(move.size(), 2);
}

TEST(StackTest, CopyOperator) {
    CustomMemoryResource custom_memory_resource;
    std::pmr::polymorphic_allocator<int> polymorphic_allocator(&custom_memory_resource);
    std::size_t capacity{20};
    CustomStack<int, std::pmr::polymorphic_allocator<int>> stack(capacity, polymorphic_allocator);
    stack.push(1);
    stack.push(2);
    CustomStack<int, std::pmr::polymorphic_allocator<int>> copy(capacity, polymorphic_allocator);
    copy = stack;
    copy.push(3);
    EXPECT_EQ(stack.top(), 2);
    EXPECT_EQ(copy.top(), 3);
}

TEST(StackTest, MoveOperator) {
    CustomMemoryResource custom_memory_resource;
    std::pmr::polymorphic_allocator<int> polymorphic_allocator(&custom_memory_resource);
    std::size_t capacity{20};
    CustomStack<int, std::pmr::polymorphic_allocator<int>> stack(capacity, polymorphic_allocator);
    stack.push(1);
    stack.push(2);
    CustomStack<int, std::pmr::polymorphic_allocator<int>> move(capacity, polymorphic_allocator);
    move = std::move(stack);
    EXPECT_TRUE(stack.empty());
    EXPECT_EQ(move.top(), 2);
    EXPECT_EQ(move.size(), 2);
}

TEST(StackTest, Iterators) {
    CustomMemoryResource custom_memory_resource;
    std::pmr::polymorphic_allocator<int> polymorphic_allocator(&custom_memory_resource);
    std::size_t capacity{10};
    CustomStack<int, std::pmr::polymorphic_allocator<int>> stack(capacity, polymorphic_allocator);
    std::vector<int> v = {1, 2, 2, 3, 3, 4, 4, 5, 5, 6};
    for (const auto& n : v) {
        stack.push(n);
    }
    EXPECT_EQ(*stack.begin(), v.front());
    EXPECT_EQ(*(stack.begin()++), v.front());
    EXPECT_EQ(*(++stack.begin()), v[1]);
    int i{0};
    for (auto it = stack.begin(); it != stack.end(); ++it) {
        EXPECT_EQ(*it, v[i]);
        ++i;
    }
}