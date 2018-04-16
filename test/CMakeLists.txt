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
