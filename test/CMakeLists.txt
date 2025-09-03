set(TEST_SOURCES
    iterator_test.cpp
    list_test.cpp
    memory_test.cpp
    utility_test.cpp
    vector_test.cpp
    algorithm/copy_test.cpp
    memory/unique_ptr_test.cpp
    memory/shared_ptr_test.cpp
    type_traits/is_integral_test.cpp
    type_traits/is_same_test.cpp
    type_traits/is_void_test.cpp
    type_traits/remove_reference_test.cpp
)

if(TEST_SOURCES)
    set(TEST_EXECUTABLE_NAME "tinystl_test")
    add_executable(${TEST_EXECUTABLE_NAME} ${TEST_SOURCES})

    # 链接 TinySTL 头文件库
    target_link_libraries(${TEST_EXECUTABLE_NAME} PRIVATE TinySTL)

    # 链接 gtest 头文件库
    target_link_libraries(${TEST_EXECUTABLE_NAME} PRIVATE GTestHeaders)

    # 链接 gtest 编译后的库
    target_link_libraries(${TEST_EXECUTABLE_NAME} PRIVATE gtest_main)

    # 添加到 CTest
    add_test(NAME ${TEST_EXECUTABLE_NAME} COMMAND ${TEST_EXECUTABLE_NAME})
else()
    message(STATUS "在 'test' 目录中没有找到测试源文件 (*.cpp)。")
endif()
