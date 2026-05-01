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

        src/core/Game.cpp
        src/core/Input.cpp

        src/entities/Player.cpp

        src/systems/BulletSystem.cpp
        src/systems/Starfield.cpp

        src/graphics/Camera.cpp
        src/graphics/Shader.cpp
        src/graphics/Texture.cpp
        src/graphics/Mesh.cpp

        src/loaders/ObjLoader.cpp
)

target_include_directories(SpaceInvaders
        PRIVATE
        src/core
        src/entities
        src/systems
        src/graphics
        src/loaders
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