add_executable(button_demo button_demo.cpp)
target_link_libraries(button_demo PRIVATE arin32)

# Keep the example warning-clean alongside the library target.
target_compile_options(button_demo PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)
