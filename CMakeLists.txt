cmake_minimum_required(VERSION 3.16)
project(main VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_PREFIX_PATH "D:/software_install_position/Qt1/6.5.3/msvc2019_64/lib/cmake")
# 自动处理 Qt 的 MOC、UIC、RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_BUILD_TYPE Debug)
# 查找 Qt 包
find_package(Qt6 REQUIRED COMPONENTS 
    Core
    Gui
    Widgets
    Network
)
aux_source_directory(. SOURCES)
# # 源文件列表
# set(SOURCES
#     main.cpp
#     tcpclient.cpp
#     protocol.cpp
#     sharefile.cpp
#     # 添加其他 .cpp 文件
# )

# 头文件列表
set(HEADERS
    tcpclient.h
    protocol.h
    sharefile.h
    # 添加其他 .h 文件
)

# 资源文件（如果有的话）
#set(RESOURCES
#    resources.qrc
#)

# 创建可执行文件
add_executable(${PROJECT_NAME} 
    ${SOURCES} 
    # ${HEADERS}
    #${RESOURCES}
)

# 链接 Qt 库
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
)

# 设置包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)