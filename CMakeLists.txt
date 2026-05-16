cmake_minimum_required(VERSION 3.14)
project(honeybee-democracy LANGUAGES CXX C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Setup FetchContent to download dependencies
include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

# Setup Eigen
FetchContent_Declare(
    Eigen
    GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
    GIT_TAG "3.4.0"
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(Eigen)

# Setup OpenGL
# cmake-format: off
cmake_policy(SET CMP0072 NEW) # Pefer GLVND over legacy GL libraries
# cmake-format: on
find_package(OpenGL REQUIRED)

# Setup GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY "https://github.com/glfw/glfw"
    GIT_TAG "3.3.8"
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glfw)

# Setup ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY "https://github.com/ocornut/imgui"
    GIT_TAG "v1.92.8-docking"
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(imgui)
set(IMGUI_SOURCE
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
add_library(imgui STATIC ${IMGUI_SOURCE})
target_include_directories(imgui PUBLIC "${imgui_SOURCE_DIR};${imgui_SOURCE_DIR}/backends/")
target_link_libraries(imgui PUBLIC glfw OpenGL::GL)

# Setup ImPlot
FetchContent_Declare(
    implot
    GIT_REPOSITORY "https://github.com/epezent/implot"
    GIT_TAG "1351ab2c46d7a05a60f3533047bfba8a953520a1"
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(implot)
set(IMPLOT_SOURCE
    ${implot_SOURCE_DIR}/implot.cpp
    ${implot_SOURCE_DIR}/implot_demo.cpp
    ${implot_SOURCE_DIR}/implot_items.cpp
)
add_library(implot STATIC ${IMPLOT_SOURCE})
target_include_directories(implot PUBLIC ${implot_SOURCE_DIR})
target_link_libraries(implot PUBLIC imgui)

# Create executable
set(HONEYBEE_SOURCE
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    ${CMAKE_SOURCE_DIR}/src/sim/bee.cpp
    ${CMAKE_SOURCE_DIR}/src/sim/hive.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/os_window.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/ui.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/windows/simulation_window.cpp
)
add_executable(honeybee_democracy ${HONEYBEE_SOURCE})
target_include_directories(honeybee_democracy PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_compile_definitions(honeybee_democracy PRIVATE HONEYBEE_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
target_link_libraries(honeybee_democracy PRIVATE Eigen3::Eigen implot)

# Silence OpenGL deprecation warnings on macOS
if(APPLE)
    target_compile_definitions(honeybee_democracy PRIVATE GL_SILENCE_DEPRECATION)
endif()
