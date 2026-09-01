option(MOLLY_ENABLE_ASAN "Enable AddressSanitizer in Debug builds" OFF)

add_library(molly_sanitizers INTERFACE)

if (MOLLY_ENABLE_ASAN)
    if (MSVC)
        target_compile_options(molly_sanitizers INTERFACE /fsanitize=address)
    else()
        target_compile_options(molly_sanitizers INTERFACE -fsanitize=address)
        target_link_options(molly_sanitizers INTERFACE -fsanitize=address)
    endif()
endif()
