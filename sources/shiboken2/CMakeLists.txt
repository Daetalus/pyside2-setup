Include(icecc.cmake)
project(shiboken2)

cmake_minimum_required(VERSION 3.1)
cmake_policy(VERSION 3.1)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules/
                      ${CMAKE_MODULE_PATH})

find_package(Qt5 5.7 REQUIRED COMPONENTS Core Xml XmlPatterns)

add_definitions(${Qt5Core_DEFINITIONS})

option(BUILD_TESTS "Build tests." TRUE)
option(USE_PYTHON_VERSION "Use specific python version to build shiboken2." "")

if (USE_PYTHON_VERSION)
    find_package(PythonInterp ${USE_PYTHON_VERSION} REQUIRED)
    find_package(PythonLibs ${USE_PYTHON_VERSION} REQUIRED)
else()
    find_package(PythonInterp 2.6)
    find_package(PythonLibs 2.6)
endif()

macro(get_python_arch)
  execute_process(
    COMMAND ${PYTHON_EXECUTABLE} -c "if True:
       import sys
       print('64' if sys.maxsize > 2**31-1 else '32')
       "
    OUTPUT_VARIABLE PYTHON_ARCH
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  message("PYTHON_ARCH:             " ${PYTHON_ARCH})
endmacro()

if (NOT PYTHON_ARCH)
    get_python_arch()
endif()

macro(get_llvm_config)
  execute_process(
    COMMAND ${PYTHON_EXECUTABLE} -c "if True:
       import os
       import sys
       sys.path.append(os.path.realpath(os.path.join('${CMAKE_CURRENT_LIST_DIR}', '..', '..')))
       from utils import findLlvmConfig
       llvmConfig = findLlvmConfig()
       if llvmConfig:
           print(llvmConfig)
       "
    OUTPUT_VARIABLE LLVM_CONFIG
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  message("LLVM_CONFIG:             " ${LLVM_CONFIG})
endmacro()

set(CLANG_DIR "")
set(CLANG_DIR_SOURCE "")

set(clang_not_found_message "Unable to detect CLANG location by checking LLVM_INSTALL_DIR, \
                             CLANG_INSTALL_DIR or running llvm-config.")

if (DEFINED ENV{LLVM_INSTALL_DIR})
    set(CLANG_DIR $ENV{LLVM_INSTALL_DIR})
    string(REPLACE "_ARCH_" "${PYTHON_ARCH}" CLANG_DIR "${CLANG_DIR}")
    set(CLANG_DIR_SOURCE "LLVM_INSTALL_DIR")
elseif (DEFINED ENV{CLANG_INSTALL_DIR})
    set(CLANG_DIR $ENV{CLANG_INSTALL_DIR})
    string(REPLACE "_ARCH_" "${PYTHON_ARCH}" CLANG_DIR "${CLANG_DIR}")
    set(CLANG_DIR_SOURCE "CLANG_INSTALL_DIR")
else ()
    if (NOT LLVM_CONFIG)
        get_llvm_config()
    endif()
    set(CLANG_DIR_SOURCE "${LLVM_CONFIG}")
    if ("${CLANG_DIR_SOURCE}" STREQUAL "")
        message(FATAL_ERROR "${clang_not_found_message}")
    endif()

    EXEC_PROGRAM("${LLVM_CONFIG}" ARGS "--prefix" OUTPUT_VARIABLE CLANG_DIR)
    if (NOT "${CLANG_DIR}" STREQUAL "")
        EXEC_PROGRAM("${LLVM_CONFIG}" ARGS "--version" OUTPUT_VARIABLE CLANG_VERSION)
        if (CLANG_VERSION VERSION_LESS 3.9)
            message(FATAL_ERROR "LLVM version 3.9 is required (${LLVM_CONFIG} detected ${CLANG_VERSION} at ${CLANG_DIR}).")
        endif()
    endif()
endif()

if ("${CLANG_DIR}" STREQUAL "")
    message(FATAL_ERROR "${clang_not_found_message}")
elseif (NOT IS_DIRECTORY ${CLANG_DIR})
    message(FATAL_ERROR "${CLANG_DIR} detected by ${CLANG_DIR_SOURCE} does not exist.")
endif()

set(CLANG_LIB_NAME "clang")
if(MSVC)
    set(CLANG_LIB_NAME "libclang")
endif()

find_library(CLANG_LIBRARY ${CLANG_LIB_NAME} HINTS ${CLANG_DIR}/lib)
if (NOT EXISTS ${CLANG_LIBRARY})
    message(FATAL_ERROR "Unable to find Clang library ${CLANG_LIB_NAME} in ${CLANG_DIR}.")
endif()

message(STATUS "CLANG: ${CLANG_DIR}, ${CLANG_LIBRARY} detected by ${CLANG_DIR_SOURCE}")

# Find highest version clang builtin includes folder to pass along to shiboken.
set(CLANG_BUILTIN_INCLUDES_DIR_PREFIX ${CLANG_DIR}/lib/clang)
file(GLOB CLANG_BUILTIN_INCLUDES_DIR_VERSIONS "${CLANG_BUILTIN_INCLUDES_DIR_PREFIX}/*")

# Sort in alphabetical order the list of version folders.
list(SORT CLANG_BUILTIN_INCLUDES_DIR_VERSIONS)

# Reverse it so the first element is the highest version.
list(REVERSE CLANG_BUILTIN_INCLUDES_DIR_VERSIONS)

message(STATUS "Found the following CLANG builtins includes directories: ${CLANG_BUILTIN_INCLUDES_DIR_VERSIONS}")
if(CLANG_BUILTIN_INCLUDES_DIR_VERSIONS)
    # Get highest version.
    list(GET CLANG_BUILTIN_INCLUDES_DIR_VERSIONS 0 CLANG_BUILTIN_INCLUDES_DIR_HIGHEST_VERSION)
    if (CLANG_BUILTIN_INCLUDES_DIR_HIGHEST_VERSION)
        # Set the final variable to the full include path to pass along to shiboken.
        set(CLANG_BUILTIN_INCLUDES_DIR "${CLANG_BUILTIN_INCLUDES_DIR_HIGHEST_VERSION}/include")
    endif()
endif()
message(STATUS "CLANG builtins includes directory chosen: ${CLANG_BUILTIN_INCLUDES_DIR}")

set(CLANG_EXTRA_INCLUDES ${CLANG_DIR}/include)
set(CLANG_EXTRA_LIBRARIES ${CLANG_LIBRARY})

set(SHIBOKEN_VERSION_FILE_PATH "${CMAKE_SOURCE_DIR}/shiboken_version.py")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
  ${SHIBOKEN_VERSION_FILE_PATH}
)
execute_process(
  COMMAND ${PYTHON_EXECUTABLE} "${SHIBOKEN_VERSION_FILE_PATH}"
  OUTPUT_VARIABLE SHIBOKEN_VERSION_OUTPUT
  ERROR_VARIABLE SHIBOKEN_VERSION_OUTPUT_ERROR
  OUTPUT_STRIP_TRAILING_WHITESPACE)
if (NOT SHIBOKEN_VERSION_OUTPUT)
    message(FATAL_ERROR "Could not identify shiboken version. \
                         Error: ${SHIBOKEN_VERSION_OUTPUT_ERROR}")
endif()

list(GET SHIBOKEN_VERSION_OUTPUT 0 shiboken_MAJOR_VERSION)
list(GET SHIBOKEN_VERSION_OUTPUT 1 shiboken_MINOR_VERSION)
list(GET SHIBOKEN_VERSION_OUTPUT 2 shiboken_MICRO_VERSION)
# a - alpha, b - beta, rc - rc
list(GET SHIBOKEN_VERSION_OUTPUT 3 shiboken_PRE_RELEASE_VERSION_TYPE)
# the number of the pre release (alpha1, beta3, rc7, etc.)
list(GET SHIBOKEN_VERSION_OUTPUT 4 shiboken_PRE_RELEASE_VERSION)

set(shiboken2_VERSION "${shiboken_MAJOR_VERSION}.${shiboken_MINOR_VERSION}.${shiboken_MICRO_VERSION}")
set(shiboken2_library_so_version "${shiboken_MAJOR_VERSION}.${shiboken_MINOR_VERSION}")

## For debugging the PYTHON* variables
message("PYTHONLIBS_FOUND: " ${PYTHONLIBS_FOUND})
message("PYTHON_LIBRARIES: " ${PYTHON_LIBRARIES})
message("PYTHON_INCLUDE_DIRS: " ${PYTHON_INCLUDE_DIRS})
message("PYTHON_DEBUG_LIBRARIES: " ${PYTHON_DEBUG_LIBRARIES})
message("PYTHONINTERP_FOUND: " ${PYTHONINTERP_FOUND})
message("PYTHON_EXECUTABLE: " ${PYTHON_EXECUTABLE})
message("PYTHON_VERSION: " ${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}.${PYTHON_VERSION_PATCH})

macro(get_python_extension_suffix)
  # Result of imp.get_suffixes() depends on the platform, but generally looks something like:
  # [('.cpython-34m-x86_64-linux-gnu.so', 'rb', 3), ('.cpython-34m.so', 'rb', 3),
  # ('.abi3.so', 'rb', 3), ('.so', 'rb', 3), ('.py', 'r', 1), ('.pyc', 'rb', 2)]
  # We pick the first most detailed one, strip of the file extension part.

  execute_process(
    COMMAND ${PYTHON_EXECUTABLE} -c "if True:
       import imp, re
       first_suffix = imp.get_suffixes()[0][0]
       res = re.search(r'^(.+)\\.', first_suffix)
       if res:
           first_suffix = res.group(1)
       else:
           first_suffix = ''
       print(first_suffix)
       "
    OUTPUT_VARIABLE PYTHON_EXTENSION_SUFFIX
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  message("PYTHON_EXTENSION_SUFFIX: " ${PYTHON_EXTENSION_SUFFIX})
endmacro()


if (NOT PYTHON_EXTENSION_SUFFIX)
  get_python_extension_suffix()
endif()

if (NOT PYTHON_CONFIG_SUFFIX)
  if (PYTHON_VERSION_MAJOR EQUAL 2)
      set(PYTHON_CONFIG_SUFFIX "-python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}")
      if (PYTHON_EXTENSION_SUFFIX)
          set(PYTHON_CONFIG_SUFFIX "${PYTHON_CONFIG_SUFFIX}${PYTHON_EXTENSION_SUFFIX}")
      endif()
  elseif (PYTHON_VERSION_MAJOR EQUAL 3)
      set(PYTHON_CONFIG_SUFFIX "${PYTHON_EXTENSION_SUFFIX}")
  endif()
endif()

if (NOT PYTHON_SHARED_LIBRARY_SUFFIX)
  set(PYTHON_SHARED_LIBRARY_SUFFIX "${PYTHON_CONFIG_SUFFIX}")

  # Append a "v" to disambiguate the python version and the shiboken version in the
  # shared library file name.
  if (APPLE AND PYTHON_VERSION_MAJOR EQUAL 2)
      set(PYTHON_SHARED_LIBRARY_SUFFIX "${PYTHON_SHARED_LIBRARY_SUFFIX}v")
  endif()
endif()


message(STATUS "PYTHON_EXTENSION_SUFFIX:      ${PYTHON_EXTENSION_SUFFIX}")
message(STATUS "PYTHON_CONFIG_SUFFIX:         ${PYTHON_CONFIG_SUFFIX}")
message(STATUS "PYTHON_SHARED_LIBRARY_SUFFIX: ${PYTHON_SHARED_LIBRARY_SUFFIX}")


if (NOT PYTHON_SITE_PACKAGES)
    execute_process(
        COMMAND ${PYTHON_EXECUTABLE} -c "if True:
            from distutils import sysconfig
            from os.path import sep
            print(sysconfig.get_python_lib(1, 0, prefix='${CMAKE_INSTALL_PREFIX}').replace(sep, '/'))
            "
        OUTPUT_VARIABLE PYTHON_SITE_PACKAGES
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (NOT PYTHON_SITE_PACKAGES)
        message(FATAL_ERROR "Could not detect Python module installation directory.")
    elseif (APPLE)
        message(STATUS "!!! The generated bindings will be installed on ${PYTHON_SITE_PACKAGES}, is it right!?")
    endif()
endif()

if(MSVC)
    # Qt5: this flag has changed from /Zc:wchar_t- in Qt4.X
    set(CMAKE_CXX_FLAGS "/Zc:wchar_t /GR /EHsc /DWIN32 /D_WINDOWS /D_SCL_SECURE_NO_WARNINGS")
else()
    if(CMAKE_HOST_UNIX AND NOT CYGWIN)
        add_definitions(-fPIC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fvisibility=hidden -Wno-strict-aliasing")
    endif()
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    option(ENABLE_GCC_OPTIMIZATION "Enable specific GCC flags to optimization library size and performance. Only available on Release Mode" 0)
    if(ENABLE_GCC_OPTIMIZATION)
        set(CMAKE_BUILD_TYPE Release)
        set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -Os -Wl,-O1")
        if(NOT CMAKE_HOST_APPLE)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--hash-style=gnu")
        endif()
    endif()
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D QT_NO_CAST_FROM_ASCII -D QT_NO_CAST_TO_ASCII")

# Force usage of the C++11 standard, without a silent fallback
# to C++98 if the compiler does not support C++11.
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(LIB_SUFFIX "" CACHE STRING "Define suffix of directory name (32/64)" )
set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}" CACHE PATH "The subdirectory relative to the install prefix where libraries will be installed (default is /lib${LIB_SUFFIX})" FORCE)

if (WIN32)
    set(PATH_SEP "\;")
else()
    set(PATH_SEP ":")
endif()

if(CMAKE_HOST_APPLE)
    set(OSX_USE_LIBCPP "OFF" CACHE BOOL "Explicitly link the libc++ standard library (useful for osx deployment targets lower than 10.9.")
    if(OSX_USE_LIBCPP)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    endif()
endif()

# Build with Address sanitizer enabled if requested. This may break things, so use at your own risk.
if (SANITIZE_ADDRESS AND NOT MSVC)
    # Currently this does not check that the clang / gcc version used supports Address sanitizer,
    # so once again, use at your own risk.
    add_compile_options("-fsanitize=address" "-g" "-fno-omit-frame-pointer")
    # We need to add the sanitize address option to all linked executables / shared libraries
    # so that proper sanitizer symbols are linked in.
    #
    # Note that when running tests, you may need to set an additional environment variable
    # in set_tests_properties for shiboken2 / pyside tests, or exported in your shell. Address
    # sanitizer will tell you what environment variable needs to be exported. For example:
    # export DYLD_INSERT_LIBRARIES=/Applications/Xcode.app/Contents/Developer/Toolchains/
    #   ./XcodeDefault.xctoolchain/usr/lib/clang/8.1.0/lib/darwin/libclang_rt.asan_osx_dynamic.dylib
    set(CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_STANDARD_LIBRARIES} -fsanitize=address")
endif()

add_subdirectory(ApiExtractor)

set(generator_plugin_DIR ${LIB_INSTALL_DIR}/generatorrunner${generator_SUFFIX})

# uninstall target
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
               IMMEDIATE @ONLY)
add_custom_target(uninstall "${CMAKE_COMMAND}"
                  -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")


# Detect if the python libs were compiled in debug mode
# On Linux distros there is no standard way to check that.
execute_process(
    COMMAND ${PYTHON_EXECUTABLE} -c "if True:
        is_py_debug = False
        import sys
        try:
            sys_py_debug = sys.pydebug
            if sys_py_debug:
                is_py_debug = True
        except:
            pass

        try:
            from distutils import sysconfig
            config_py_debug = sysconfig.get_config_var('Py_DEBUG')
            if config_py_debug:
                is_py_debug = True
        except:
            pass

        print(bool(is_py_debug))
        "
    OUTPUT_VARIABLE PYTHON_WITH_DEBUG
    OUTPUT_STRIP_TRAILING_WHITESPACE)

# Detect if python interpeter was compiled with COUNT_ALLOCS define
# Linux distros are inconsistent in setting the sysconfig.get_config_var('COUNT_ALLOCS') value
execute_process(
    COMMAND ${PYTHON_EXECUTABLE} -c "if True:
        count_allocs = False
        import sys
        try:
            if sys.getcounts:
                count_allocs = True
        except:
            pass

        print(bool(count_allocs))
        "
    OUTPUT_VARIABLE PYTHON_WITH_COUNT_ALLOCS
    OUTPUT_STRIP_TRAILING_WHITESPACE)

set(SHIBOKEN_BUILD_TYPE "Release")

# We do not want to link against the python shared / static library on Linux And macOS.
# The dynamic linker will pick up the python symbols at runtime automatically.
# On Windows we do need to link against the python.lib import library.

set(SBK_PYTHON_LIBRARIES "")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(NOT PYTHON_DEBUG_LIBRARIES)
        message(WARNING "Python debug shared library not found; assuming python was built with shared library support disabled.")
    endif()

    if(NOT PYTHON_WITH_DEBUG)
        message(WARNING "Compiling shiboken2 with debug enabled, but the python executable was not compiled with debug support.")
    else()
        add_definitions("-DPy_DEBUG")
        set(SBK_ADD_PY_DEBUG_DEFINITION "add_definitions(\"-DPy_DEBUG\")")
        set(SBK_PKG_CONFIG_PY_DEBUG_DEFINITION " -DPy_DEBUG")
    endif()

    if (PYTHON_WITH_COUNT_ALLOCS)
        add_definitions("-DCOUNT_ALLOCS")
        set(SBK_ADD_PY_DEBUG_DEFINITION "${SBK_ADD_PY_DEBUG_DEFINITION} \nadd_definitions(\"-DCOUNT_ALLOCS\")")
        set(SBK_PKG_CONFIG_PY_DEBUG_DEFINITION "${SBK_PKG_CONFIG_PY_DEBUG_DEFINITION} -DCOUNT_ALLOCS")
    endif()

    if(WIN32)
        set(SBK_PYTHON_LIBRARIES ${PYTHON_DEBUG_LIBRARIES})
    endif()
    set(SHIBOKEN_BUILD_TYPE "Debug")
else()
    if(WIN32)
        set(SBK_PYTHON_LIBRARIES ${PYTHON_LIBRARIES})
    endif()
    add_definitions("-DNDEBUG")
endif()

if(APPLE)
    set(SBK_PYTHON_LIBRARIES "-undefined dynamic_lookup")
endif()

if(CMAKE_VERSION VERSION_LESS 2.8)
    set(SBK_PYTHON_INCLUDE_DIR ${PYTHON_INCLUDE_PATH})
else()
    set(SBK_PYTHON_INCLUDE_DIR ${PYTHON_INCLUDE_DIRS})
endif()

add_subdirectory(libshiboken)
add_subdirectory(doc)

# deps found, compile the generator.
if (Qt5Core_FOUND AND PYTHONINTERP_FOUND)
    add_subdirectory(generator)
    add_subdirectory(shibokenmodule)

    if (BUILD_TESTS)
        enable_testing()
        add_subdirectory(tests)
    endif()
else()
    message(WARNING "Some dependencies were not found, shiboken2 generator compilation disabled!")
endif()

add_subdirectory(data)
