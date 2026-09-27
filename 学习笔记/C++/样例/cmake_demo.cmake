cmake_minimum_required(VERSION 3.10) #版本要求
project(ThreadPoolProject)           #项目名称

set(CMAKE_CXX_STANDARD 20)           #常规设置标准
set(CMAKE_CXX_STANDARD_REQUIRED ON)  #强制要求标准  不支持则无法编译
set(CMAKE_CXX_OPTIMIZE_LEVEL 0)      #优化级别

#设置输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/libs) # 静态库 (.a)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/libs) # 动态库 (.so)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)  # 可执行文件

#把这个目录加入到当前 CMake 目录以及其子目录中创建的 target 的头文件搜索路径里
#通常更建议使用target_include_directories
include_directories(${CMAKE_SOURCE_DIR}/tools/includes)

#在PATH中寻找这个库的.cmake  大型库找COMPONENTS
#find_package(<PackageName>
#    [version]
#    [REQUIRED]
#    [COMPONENTS component1 component2 ...]
#)
find_package(jsoncpp REQUIRED)
#编译库文件  静态 STATIC  动态 SHARED
add_library(ThreadPool STATIC tools/ThreadPool.cc)
add_library(JsonParser STATIC tools/JsonParser.cc)
add_library(MemoryPool STATIC tools/MemoryPool.cc)

#生成可执行文件
add_executable(test test.cc) 
add_executable(AsyncLogger tools/AsyncLogger.cc)

#链接库 PRIVATE不传导依赖  PUBLIC传导依赖  INTERFACE自己不使用，也传导依赖
#库A用到另一个库
# target_link_libraries(test PRIVATE jsoncpp_lib)
# include(xxx/demo.cmake)   #导入cmake

#条件设置宏
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_definitions(test PUBLIC
        $<$<PLATFORM_ID:Windows>:MYNAME="Bill Gates">
        $<$<PLATFORM_ID:Linux>:MYNAME="Linus">
        $<$<PLATFORM_ID:Darwin>:MYNAME="Steven Jobs">
    )
endif()