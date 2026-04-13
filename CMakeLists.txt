cmake_minimum_required(VERSION 4.0)
project(CefTask)

set(CMAKE_CXX_STANDARD 20)

add_executable(CefTask  main.cpp
        src/base/instance.hpp
        src/base/instance.cpp
        src/base/base.hpp
        src/base/base.cpp
        src/base/texture.hpp
        src/base/texture.cpp
        src/app/app.hpp
        src/app/app.cpp
        src/app/ffmpeg.hpp
        src/app/ffmpeg.cpp
        src/app/cef.hpp
        src/app/cef.cpp
        src/backend/backend.hpp
        )

set(CEF_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third_party/cef")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CEF_ROOT}/cmake")
        
if (UNIX)
    target_sources(CefTask PRIVATE 
    src/backend/LinuxX11/x11.cpp)
    find_package(X11 REQUIRED)
    find_package(OpenGL REQUIRED COMPONENTS EGL GLES2)
    find_package(PkgConfig REQUIRED)
    find_package(CEF REQUIRED)
    pkg_check_modules(AVCODEC REQUIRED IMPORTED_TARGET libavcodec)
    pkg_check_modules(AVUTIL REQUIRED IMPORTED_TARGET libavutil)
    pkg_check_modules(AVFORMAT REQUIRED IMPORTED_TARGET libavformat)

    SET_CEF_TARGET_OUT_DIR()

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        add_compile_options(-Wno-character-conversion)
    endif()

    set_target_properties(CefTask PROPERTIES INSTALL_RPATH "$ORIGIN")
    set_target_properties(CefTask PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
    set_target_properties(CefTask PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CEF_TARGET_OUT_DIR})


    message(STATUS "CEF Include Path: ${CEF_ROOT}")
    add_subdirectory(${CEF_ROOT} libcef_dll_wrappers)
    target_include_directories(CefTask PRIVATE 
    ${X11_INCLUDE_DIR}
    ${CEF_ROOT}
    )

    link_directories(${CEF_ROOT}/Debug)

    target_link_libraries(CefTask PRIVATE 
        ${X11_LIBRARIES}
        PkgConfig::AVCODEC
        PkgConfig::AVUTIL
        PkgConfig::AVFORMAT 
        OpenGL::EGL 
        OpenGL::GLES2
        ${CEF_STANDARD_LIBS}
        "${CEF_ROOT}/Debug/libcef.so"
        libcef_dll_wrapper 
    )

endif()


add_custom_command(
    TARGET CefTask POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
            "${CEF_TARGET_OUT_DIR}/shaders"
    COMMENT "Copying ${RES_FILE} to build directory"
)

add_custom_command(
    TARGET CefTask POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/assets"
            "${CEF_TARGET_OUT_DIR}/assets"
    COMMENT "Copying ${RES_FILE} to build directory"
)

COPY_FILES(CefTask "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "${CEF_TARGET_OUT_DIR}")
COPY_FILES(CefTask "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "${CEF_TARGET_OUT_DIR}")
if (EXISTS "${CEF_BINARY_DIR}/libminigbm.so")
COPY_FILES(CefTask "libminigbm.so" "${CEF_BINARY_DIR}" "${CEF_TARGET_OUT_DIR}")
endif()

SET_LINUX_SUID_PERMISSIONS(CefTask $<TARGET_FILE_DIR:CefTask>/chrome-sandbox)
