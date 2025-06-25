cmake_minimum_required(VERSION 3.22)
# 设定项目名称
set(CMAKE_C_COMPILER "D:/mingw64/bin/gcc.exe")
set(CMAKE_CXX_COMPILER "D:/mingw64/bin/g++.exe")
project(dots_and_boxes)
# 设定语言标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
IF(WIN32)
# 定义第三方库路径
set(SFML_DIR 3rdparty/SFML/lib/cmake/SFML)
ENDIF()
set(nowide_DIR 3rdparty/nowide/lib/cmake/nowide)
# 静态链接SFML
#set(SFML_STATIC_LIBRARIES TRUE)

# 查找第三方库
find_package(SFML 2.5 COMPONENTS
        system window graphics REQUIRED)
find_package(nowide)

file(GLOB_RECURSE SOURCES
        src/*.cpp
        src/*.c
        src/*.h
)
add_executable(dots_and_boxes ${SOURCES})

target_link_libraries(dots_and_boxes
    PRIVATE
        nowide::nowide
)


# 链接头文件查找目录
IF(WIN32)
target_include_directories(dots_and_boxes PUBLIC 3rdparty/SFML/include)
ENDIF()
target_include_directories(dots_and_boxes PUBLIC 3rdparty/nowide/include)

target_link_libraries(dots_and_boxes PRIVATE sfml-system sfml-window sfml-graphics)

IF (WIN32)
file(GLOB_RECURSE PROJECT_DLL 3rdparty/*.dll)
add_custom_command(TARGET dots_and_boxes POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${PROJECT_DLL}
        $<TARGET_FILE_DIR:dots_and_boxes>
        )
ENDIF ()

# 复制资源文件
add_custom_command(TARGET dots_and_boxes POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/res
        $<TARGET_FILE_DIR:dots_and_boxes>/res
        )

# 重新生成并编译
execute_process(COMMAND rm -r build)
execute_process(COMMAND cmake -B build)
execute_process(COMMAND cmake --build build --config Release)