cmake_minimum_required(VERSION 3.8)

project(filemergesort LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_REQUIRED_FLAGS ${CMAKE_CXX17_STANDARD_COMPILE_OPTION})

add_executable(filemergesort main.cpp open_file.cpp open_file.h tmp_file.cpp tmp_file.h cancel.h cancel.cpp)
add_executable(gentest EXCLUDE_FROM_ALL gentest.cpp open_file.cpp open_file.h)

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-Wall CXX_COMPILER_SUPPORTS_WALL)

if(CXX_COMPILER_SUPPORTS_WALL)
    target_compile_options(filemergesort PRIVATE -Wall)
    target_compile_options(gentest PRIVATE -Wall)
endif()

check_cxx_source_compiles("#include <filesystem>\nint main() {}" HAS_STD_FILESYSTEM)

if(HAS_STD_FILESYSTEM)
    target_compile_definitions(filemergesort PRIVATE USE_STD_FILESYSTEM)
else()
    target_link_libraries(filemergesort PRIVATE stdc++fs)
endif()

add_custom_target(test-small)

foreach(TEST_SIZE 0 1 2 10 1000 10000)
    add_custom_command(
        OUTPUT test${TEST_SIZE}.input test${TEST_SIZE}.expected_output
        COMMAND gentest ${TEST_SIZE} test${TEST_SIZE}.input test${TEST_SIZE}.expected_output
        DEPENDS gentest
    )

    add_custom_target(testdata-${TEST_SIZE} DEPENDS test${TEST_SIZE}.input test${TEST_SIZE}.expected_output)

    foreach(CHUNK_SIZE 1 2 10 100 1000 10000)
        add_custom_command(
            OUTPUT test${TEST_SIZE}_${CHUNK_SIZE}.output
            COMMAND filemergesort test${TEST_SIZE}.input test${TEST_SIZE}_${CHUNK_SIZE}.output ${CHUNK_SIZE}
            DEPENDS testdata-${TEST_SIZE} test${TEST_SIZE}.input filemergesort
        )

        add_custom_target(
            test_${TEST_SIZE}_${CHUNK_SIZE}
            COMMAND diff -q test${TEST_SIZE}_${CHUNK_SIZE}.output test${TEST_SIZE}.expected_output
            DEPENDS test${TEST_SIZE}_${CHUNK_SIZE}.output testdata-${TEST_SIZE}
        )

        add_dependencies(test-small test_${TEST_SIZE}_${CHUNK_SIZE})
    endforeach()
endforeach()

set(BIG_TEST_SIZE 500000000)
set(BIG_CHUNK_SIZE 4000)

add_custom_command(
    OUTPUT test${BIG_TEST_SIZE}.input
    COMMAND gentest ${BIG_TEST_SIZE} test${BIG_TEST_SIZE}.input
    DEPENDS gentest
)

add_custom_target(testdata-${BIG_TEST_SIZE} DEPENDS test${BIG_TEST_SIZE}.input)

add_custom_command(
    OUTPUT test${BIG_TEST_SIZE}_${BIG_CHUNK_SIZE}.output
    COMMAND filemergesort test${BIG_TEST_SIZE}.input test${BIG_TEST_SIZE}_${BIG_CHUNK_SIZE}.output ${BIG_CHUNK_SIZE}
    DEPENDS testdata-${BIG_TEST_SIZE} test${BIG_TEST_SIZE}.input filemergesort
)

add_custom_target(
    test-big
    DEPENDS test${BIG_TEST_SIZE}_${BIG_CHUNK_SIZE}.output testdata-${BIG_TEST_SIZE}
)

add_custom_target(check DEPENDS test-small test-big)
