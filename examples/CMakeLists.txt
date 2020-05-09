cmake_minimum_required(VERSION 3.0)
project(liblifthttp_examples CXX)

### readme ###
add_executable(lift_readme readme.cpp)
target_link_libraries(lift_readme PRIVATE lifthttp)

### synch_simple ###
add_executable(lift_synch_simple synch_simple.cpp)
target_link_libraries(lift_synch_simple PRIVATE lifthttp)

# ### async_simple ###
add_executable(lift_async_simple async_simple.cpp)
target_link_libraries(lift_async_simple PRIVATE lifthttp)

# ### benchmark ###
add_executable(lift_benchmark benchmark.cpp)
target_link_libraries(lift_benchmark PRIVATE lifthttp)
