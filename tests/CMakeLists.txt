find_package(GTest REQUIRED)

add_executable(arin_tests
    test_types.cpp
    test_button.cpp
    test_input.cpp
)

target_link_libraries(arin_tests PRIVATE
    arin32
    GTest::gtest
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(arin_tests)
