# Benchmark executable
add_executable(benchmark_runner
  benchmark.cpp
)

# Link against your sorting_algorithms library
target_link_libraries(benchmark_runner
  PRIVATE sorting_algorithms utils
)

# Ensure it also sees the public headers
target_include_directories(benchmark_runner
  PRIVATE ${PROJECT_SOURCE_DIR}/src/include
)

#  Place the built binary into a central ../bin folder:
set_target_properties(benchmark_runner
 PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
