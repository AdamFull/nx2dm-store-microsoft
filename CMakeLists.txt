
option(NX_MODULE_STORE_MICROSOFT "Build the Microsoft Store (Windows.Services.Store) storefront backend" OFF)

if (NOT NX_MODULE_STORE_MICROSOFT)
    return()
endif ()

# nx_module()'s own PLATFORMS gate runs too late to help here - a project
# that also builds for Android/iOS still passes
# -DNX_MODULE_STORE_MICROSOFT=ON for this platform-irrelevant module (its
# module list isn't per-platform), so this must skip itself before ever
# reaching that gate, the same "skipped on <platform>" outcome
# _nx_module_gate would otherwise report.
if (NOT WIN32)
    message(STATUS "nx2d: module 'store_microsoft' skipped on this platform")
    return()
endif ()

# No vendor SDK to resolve here, unlike every other store backend - the
# C++/WinRT projection headers (winrt/Windows.Services.Store.h etc) already
# ship inside the installed Windows 10/11 SDK
# (<SDK>/Include/<version>/cppwinrt), on the compiler's default include
# path already. The one import library this needs (windowsapp.lib) is
# pulled in via #pragma comment(lib, ...) directly in
# store_microsoft_platform.cpp instead of DEPENDS here, since it is a
# plain system library, not a target this repo owns or vendors.
nx_module(store_microsoft
        DISPLAY "Microsoft Store (Windows.Services.Store) storefront backend"
        SOURCES store_microsoft_module.cpp store_microsoft_platform.cpp
                store_microsoft_services.cpp
        DEPENDS nx_module_store
        PLATFORMS windows)

if (NX_BUILD_TESTS)
    add_subdirectory(tests)
endif ()
