add_executable(hashtable_examples hashtable_examples.cpp)
target_link_libraries(hashtable_examples PRIVATE TinySTL)

add_executable(unordered_map_examples unordered_map_examples.cpp)
target_link_libraries(unordered_map_examples PRIVATE TinySTL)
