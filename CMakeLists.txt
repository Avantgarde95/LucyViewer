cmake_minimum_required(VERSION 3.8)
project(ModelViewer)

set(CMAKE_CXX_STANDARD 11)

if (MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif ()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(embree 3.0 REQUIRED)
find_package(TBB REQUIRED tbb)

find_package(
        Qt5 REQUIRED COMPONENTS
        Core
        Concurrent
        Gui
        Widgets
)

set(APP_TARGET ModelViewer)

set(
        APP_SOURCES

        src/base/Object.cpp

        src/model/SceneModel.cpp
        src/model/FPSModel.cpp

        src/view/StatusView.cpp
        src/view/ObjectsView.cpp
        src/view/PixelsView.cpp
        src/view/WindowView.cpp

        src/App.cpp
        src/Main.cpp
)

add_executable(${APP_TARGET} ${APP_SOURCES})

target_include_directories(
        ${APP_TARGET} PUBLIC
        ${EMBREE_INCLUDE_DIRS}
        3rd/glm
        3rd/tinyply/source
        3rd/flythrough_camera
)

target_link_libraries(
        ${APP_TARGET}
        ${EMBREE_LIBRARY}
        ${TBB_IMPORTED_TARGETS}
        Qt5::Core
        Qt5::Concurrent
        Qt5::Gui
        Qt5::Widgets
)

if (MSVC)
    file(GLOB EMBREE_DLLS ${EMBREE_ROOT_DIR}/bin/embree*.dll ${EMBREE_ROOT_DIR}/bin/tbb*.dll)
else ()
    set(EMBREE_DLLS "")
endif ()

add_custom_command(
        TARGET ${APP_TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${EMBREE_DLLS} $<TARGET_FILE_DIR:${APP_TARGET}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Core> $<TARGET_FILE_DIR:${APP_TARGET}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Concurrent> $<TARGET_FILE_DIR:${APP_TARGET}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Gui> $<TARGET_FILE_DIR:${APP_TARGET}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Widgets> $<TARGET_FILE_DIR:${APP_TARGET}>
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/res $<TARGET_FILE_DIR:${APP_TARGET}>/res
)
