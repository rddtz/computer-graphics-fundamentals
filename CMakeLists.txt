cmake_minimum_required(VERSION 3.5)
project(LAB_FCG VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Source files
set(SOURCES
    src/main.cpp
    src/textrendering.cpp
    src/tiny_obj_loader.cpp
    src/stb_image.cpp
    src/glad.c
)

# Output directories
if(WIN32)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${PROJECT_SOURCE_DIR}/bin/Debug")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_SOURCE_DIR}/bin/Release")
elseif(UNIX)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/Linux")
endif()

# Executable
add_executable(main ${SOURCES})

target_include_directories(main PRIVATE ${PROJECT_SOURCE_DIR}/include)

if(WIN32)
    if(MINGW)
        # Always use 64-bit MinGW GLFW library
        set(LIBGLFW ${PROJECT_SOURCE_DIR}/libs/lib-mingw-64/libglfw3.a)
    elseif(MSVC)
        set(LIBGLFW ${PROJECT_SOURCE_DIR}/libs/lib-vc2022/glfw3.lib)
    else()
        message(FATAL_ERROR "This CMakeLists.txt only supports MinGW or MSVC on Windows.")
    endif()
    target_link_libraries(main ${LIBGLFW} gdi32 opengl32)
elseif(UNIX)
    target_compile_options(main PRIVATE -Wall -Wno-unused-function)
    find_package(OpenGL REQUIRED)
    find_package(X11 REQUIRED)
    find_library(MATH_LIBRARY m)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    target_link_libraries(main
        ${CMAKE_DL_LIBS}
        ${MATH_LIBRARY}
        ${PROJECT_SOURCE_DIR}/libs/lib-linux/libglfw3.a
        ${CMAKE_THREAD_LIBS_INIT}
        ${OPENGL_LIBRARIES}
        ${X11_LIBRARIES}
        ${X11_Xrandr_LIB}
        ${X11_Xcursor_LIB}
        ${X11_Xinerama_LIB}
        ${X11_Xxf86vm_LIB}
    )
    add_custom_target(run
        COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} ./main
        DEPENDS main
        USES_TERMINAL
    )
endif()
