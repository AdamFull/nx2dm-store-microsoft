
add_executable(nx_store_microsoft_tests
        test_store_microsoft_module.cpp
        test_store_microsoft_platform.cpp
        test_store_microsoft_services.cpp
        test_store_microsoft_rate_review.cpp
        test_store_microsoft_scripting.cpp
        ../store_microsoft_module.cpp
        ../store_microsoft_platform.cpp
        ../store_microsoft_services.cpp
        ../store_microsoft_rate_review.cpp
        ../store_microsoft_scripting.cpp)

nx_module_registry(nx_store_microsoft_tests)

target_include_directories(nx_store_microsoft_tests PRIVATE
        "${NX_ENGINE_ROOT}/engine" "${NX_ENGINE_ROOT}/modules")

target_link_libraries(nx_store_microsoft_tests
        PRIVATE nx::test nx::foundation nx::engine nx::compile_options)

set_target_properties(nx_store_microsoft_tests PROPERTIES FOLDER "tests")
nxtest_discover_tests(nx_store_microsoft_tests LABELS store_microsoft)
