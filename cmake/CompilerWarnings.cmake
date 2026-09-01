add_library(molly_warnings INTERFACE)

if (MSVC)
    target_compile_options(molly_warnings INTERFACE /W4 /permissive-)
else()
    target_compile_options(molly_warnings INTERFACE -Wall -Wextra -Wpedantic)
endif()
