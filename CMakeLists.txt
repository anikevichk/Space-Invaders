cmake_minimum_required(VERSION 3.20)

project(SpaceInvaders)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(tinyobjloader CONFIG REQUIRED)

find_path(Stb_INCLUDE_DIR NAMES stb_image.h)

add_executable(SpaceInvaders
        main.cpp
        src/Shader.cpp
        src/ObjLoader.cpp
        src/Texture.cpp
        src/Mesh.cpp
        src/Input.cpp
)

target_include_directories(SpaceInvaders
        PRIVATE
        src
        ${Stb_INCLUDE_DIR}
)

target_link_libraries(SpaceInvaders
        PRIVATE
        OpenGL::GL
        glfw
        GLEW::GLEW
        glm::glm
        tinyobjloader::tinyobjloader
)

file(COPY assets shaders DESTINATION ${CMAKE_BINARY_DIR})