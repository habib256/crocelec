cmake_minimum_required(VERSION 3.16)
project(crocelec VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# ─── Dear ImGui (branche docking, commit épinglé dans imgui_pin.env) ──────
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
if(NOT EXISTS "${IMGUI_DIR}/imgui.cpp")
    message(FATAL_ERROR "Dear ImGui absent de ${IMGUI_DIR}. Lance ./setup_imgui.sh d'abord.")
endif()

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

set(CROCELEC_SOURCES
    src/main.cpp
    src/App.cpp
    src/Circuit.cpp
    src/Solver.cpp
    src/Schematic.cpp
    src/Panels.cpp
    src/Instruments.cpp
    src/Exemples.cpp
    src/Format.cpp
    src/Serialize.cpp
)

add_executable(crocelec ${CROCELEC_SOURCES} ${IMGUI_SOURCES})
target_include_directories(crocelec PRIVATE ${IMGUI_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src)

# ─── GLFW + OpenGL ────────────────────────────────────────────────────────
find_package(OpenGL REQUIRED)
target_link_libraries(crocelec PRIVATE OpenGL::GL)

find_package(glfw3 QUIET)
if(glfw3_FOUND)
    target_link_libraries(crocelec PRIVATE glfw)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLFW REQUIRED glfw3)
    target_include_directories(crocelec PRIVATE ${GLFW_INCLUDE_DIRS})
    target_link_libraries(crocelec PRIVATE ${GLFW_LIBRARIES})
    target_link_directories(crocelec PRIVATE ${GLFW_LIBRARY_DIRS})
endif()

if(APPLE)
    target_link_libraries(crocelec PRIVATE "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
endif()

if(MSVC)
    target_compile_options(crocelec PRIVATE /W3 /utf-8)
else()
    target_compile_options(crocelec PRIVATE -Wall -Wextra -Wno-unused-parameter)
endif()

# ─── Tests de recette du solveur (§ 11 du cahier des charges) ─────────────
# Sans interface : Circuit + Solver + Exemples ne dépendent pas d'ImGui, ce
# qui rend la suite instantanée et utilisable en CI.
enable_testing()
add_executable(crocelec_tests
    tests/test_solveur.cpp
    src/Circuit.cpp
    src/Solver.cpp
    src/Exemples.cpp
    src/Format.cpp
    src/Serialize.cpp
)
target_include_directories(crocelec_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_test(NAME solveur COMMAND crocelec_tests)
