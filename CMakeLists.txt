cmake_minimum_required(VERSION 3.10)
project(CanteenManagementSystem)
 
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies
find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(OpenSSL REQUIRED)

# ImGui sources
file(GLOB IMGUI_SOURCES imgui/*.cpp)
list(APPEND IMGUI_SOURCES
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)

# CanteenManagementSystem executable
add_executable(CanteenManagementSystem
    main.cpp
    ${IMGUI_SOURCES}
)
target_include_directories(CanteenManagementSystem PRIVATE
    ${CMAKE_SOURCE_DIR}/imgui
    ${CMAKE_SOURCE_DIR}/imgui/backends
    ${SQLite3_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
)
target_link_libraries(CanteenManagementSystem PRIVATE
    OpenGL::GL
    glfw
    ${SQLite3_LIBRARIES}
    ${OPENSSL_LIBRARIES}
)

# AdminPanel executable
add_executable(AdminPanel
    admin.cpp
)
target_include_directories(AdminPanel PRIVATE
    ${SQLite3_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
)
target_link_libraries(AdminPanel PRIVATE
    ${SQLite3_LIBRARIES}
    ${OPENSSL_LIBRARIES}
)
