# Build a static library for all sorting algorithms
add_library(utils
  timer.cpp
)

# Make headers in src/include/ visible to anyone linking this lib
target_include_directories(utils
  PUBLIC
    ${PROJECT_SOURCE_DIR}/src/include
)

# (Optional) If you want warnings or extra flags per-target:
# target_compile_options(sorting_algorithms PRIVATE -Wall -Wextra)
