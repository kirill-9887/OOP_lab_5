#include "./include/custom_memory_resource.hpp"
#include "./include/custom_stack.hpp"

int main() {
    std::cout << "\n1. Создание пользовательского memory_resource:" << std::endl;
    CustomMemoryResource custom_memory_resource;
    std::pmr::polymorphic_allocator<int> polymorphic_allocator(&custom_memory_resource);
    std::cout << "   Полиморфный аллокатор создан" << std::endl;
    
    std::cout << "\n2. Создание стека с пользовательской структурой:" << std::endl;
    struct TestStructure {
        int first_value;
        int second_value;
    };

    CustomStack<TestStructure, std::pmr::polymorphic_allocator<TestStructure>> 
        structure_stack(10, std::pmr::polymorphic_allocator<TestStructure>(&custom_memory_resource));
    std::cout << "   Стек структур создан" << std::endl;

    std::cout << "\n3. Создание множественных стеков:" << std::endl;
    CustomStack<int, std::pmr::polymorphic_allocator<int>> *custom_stack1, *custom_stack2, *custom_stack3, *custom_stack4;

    custom_stack1 = new CustomStack<int, std::pmr::polymorphic_allocator<int>>(10, polymorphic_allocator);
    custom_stack2 = new CustomStack<int, std::pmr::polymorphic_allocator<int>>(10, polymorphic_allocator);
    custom_stack3 = new CustomStack<int, std::pmr::polymorphic_allocator<int>>(10, polymorphic_allocator);
    custom_stack4 = new CustomStack<int, std::pmr::polymorphic_allocator<int>>(10, polymorphic_allocator);
    std::cout << "   Четыре стека созданы" << std::endl;

    std::cout << "\n4. Освобождение части стеков:" << std::endl;
    delete custom_stack2;
    delete custom_stack3;
    std::cout << "   Два стека освобождены" << std::endl;

    std::cout << "\n5. Циклическое создание и использование стеков:" << std::endl;
    for (int iteration = 0; iteration < 10; ++iteration) {
        std::cout << "   Итерация " << iteration << ":" << std::endl;
        
        CustomStack<int, std::pmr::polymorphic_allocator<int>> temp_stack(10, polymorphic_allocator);
        CustomStack<int, std::pmr::polymorphic_allocator<int>> large_stack(40, polymorphic_allocator);
        
        for (int element_index = 0; element_index < 10; ++element_index) {
            temp_stack.push(element_index);
        }
        
        std::cout << "     Содержимое: ";
        for (int element_value : temp_stack) {
            std::cout << element_value << " ";
        }
        std::cout << std::endl;
    }

    std::cout << "\n6. Освобождение оставшихся стеков:" << std::endl;
    delete custom_stack1;
    delete custom_stack4;
    std::cout << "   Все стеки освобождены" << std::endl;

    return 0;
}