cmake_minimum_required(VERSION 3.0.0)
project(PxlRnd VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)

include(CTest)
enable_testing()

add_subdirectory(deps/glad)

add_executable(
    PxlRnd
    src/main.cpp
    src/Renderer.h
    src/SpriteBatch.h
    src/GLRenderer.cpp src/GLRenderer.h
)

if(EMSCRIPTEN)
    set_target_properties(PxlRnd PROPERTIES LINK_FLAGS "-O2 -sASYNCIFY -sUSE_WEBGL2=1 -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sUSE_SDL=2 -sUSE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='\"png\"' --preload-file ../res --profiling")
endif()

if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBCMT")
endif()

# Save the length of the source path, which is used to truncate the __FILE__ macro.
string(LENGTH "${CMAKE_SOURCE_DIR}/" SOURCE_PATH_SIZE)
add_definitions("-DSOURCE_PATH_SIZE=${SOURCE_PATH_SIZE}")

# Find packages, the following line is only necessary for VCPKG users.
set(VCPKG_TARGET_TRIPLET x64-windows-static)
include($ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)

if(EMSCRIPTEN)
    find_package(SDL2 CONFIG REQUIRED)
    find_package(glm CONFIG REQUIRED)

    target_link_libraries(
        PxlRnd PRIVATE
        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
        glm::glm
        glad
    )
else()
    find_package(SDL2 CONFIG REQUIRED)
    find_package(SDL2_image CONFIG REQUIRED)
    find_package(glm CONFIG REQUIRED)

    target_link_libraries(
        PxlRnd PRIVATE
        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
        $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>
        glm::glm
        glad
    )
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
