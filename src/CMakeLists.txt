# Add the main executable
add_executable(LowLatencyExecutionEngine
    main.cpp
    parsing/MessageParser.cpp
    parsing/MessageBuilder.cpp
    benchmarking/LatencyTracker.cpp
    # Add other .cpp files here if needed
)

# Include the top-level include folder if you have headers
target_include_directories(LowLatencyExecutionEngine PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(LowLatencyExecutionEngine PRIVATE ws2_32)

# Compiler flags for optimization
target_compile_options(LowLatencyExecutionEngine PRIVATE
    $<$<CONFIG:Release>:-O3 -march=native -flto>
)