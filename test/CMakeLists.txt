project(test)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_subdirectory(gtest)

if( MSVC ) # VS2012 doesn't support correctly the tuples yet
    add_definitions( /D _VARIADIC_MAX=10 )
endif()
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(runTests ../lab3.cpp ../lab3.h tests.cpp)

target_link_libraries(runTests gtest gtest_main)
target_link_libraries(runTests)

# see https://crascit.com/2015/03/28/enabling-cxx11-in-cmake/
set_target_properties(runTests
    PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
)
