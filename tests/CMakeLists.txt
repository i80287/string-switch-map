cmake_minimum_required(VERSION 3.5)

project(math_tests VERSION 0.1.0 LANGUAGES CXX)

# Empty by default
set(TEST_COMPILE_OPTIONS)
set(TEST_COMPILE_DEFINITIONS)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    elseif (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
        if (NOT (CMAKE_BUILD_TYPE STREQUAL "Debug"))
            set(TEST_COMPILE_OPTIONS ${TEST_COMPILE_OPTIONS} -UNDEBUG)
        endif()
        set(TEST_COMPILE_DEFINITIONS
            ${TEST_COMPILE_DEFINITIONS}
            _LIBCPP_ENABLE_ASSERTIONS=1)
        
    set(TEST_COMPILE_OPTIONS
    ${TEST_COMPILE_OPTIONS}
        -fstack-protector-strong
        -mshstk
        -fno-omit-frame-pointer
        -Wall
        -Wextra
        -Wcast-qual
        -Wpedantic
        -Wshift-overflow
        -Wunused
        -Wconversion
        -Wshadow
        -Wnull-dereference
        -Wundef
        -Wwrite-strings
        -Wsign-conversion
        -Wmissing-noreturn
        -Wunreachable-code
        -Wcast-align
        -Warray-bounds
        -Wformat=2
    )
    elseif (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "AppleClang")
    endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (NOT (CMAKE_BUILD_TYPE STREQUAL "Debug"))
        set(TEST_COMPILE_OPTIONS ${TEST_COMPILE_OPTIONS} -UNDEBUG)
    endif()
    set(TEST_COMPILE_OPTIONS
        ${TEST_COMPILE_OPTIONS}
        -fstack-protector-strong
        -mshstk
        -fno-omit-frame-pointer
        -Wall
        -Wextra
        -Wlogical-op
        -Wcast-qual
        -Wpedantic
        -Wshift-overflow=2
        -Wduplicated-cond
        -Wunused
        -Wconversion
        -Wunsafe-loop-optimizations
        -Wshadow
        -Wnull-dereference
        -Wundef
        -Wwrite-strings
        -Wsign-conversion
        -Wmissing-noreturn
        -Wunreachable-code
        -Wcast-align
        -Warray-bounds=2
        -Wformat=2)
    set(TEST_COMPILE_DEFINITIONS
        ${TEST_COMPILE_DEFINITIONS}
        _GLIBCXX_DEBUG=1
        _GLIBCXX_DEBUG_PEDANTIC=1
        _GLIBCXX_CONCEPT_CHECKS=1)
endif()

set(target_filename "tests")

foreach(cxx_version 20 23)
    string(CONCAT target_cpp_filename ${target_filename} ".cpp")
    string(CONCAT cmake_target_name ${target_filename} "_cxx_" ${cxx_version})

    add_executable(${cmake_target_name} ${target_cpp_filename})
    target_compile_options(${cmake_target_name} PRIVATE ${TEST_COMPILE_OPTIONS})
    target_compile_definitions(${cmake_target_name} PRIVATE ${TEST_COMPILE_DEFINITIONS})

    set_target_properties(${cmake_target_name} PROPERTIES CXX_STANDARD ${cxx_version} CXX_EXTENSIONS OFF CXX_STANDARD_REQUIRED ON)
    add_test(NAME ${cmake_target_name} COMMAND $<TARGET_FILE:${cmake_target_name}>)
endforeach()

enable_testing()
