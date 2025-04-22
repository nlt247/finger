cmake_minimum_required(VERSION 3.10)
project(fingerprint_lib VERSION 1.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置构建动态库
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_definitions(-DBUILDING_DLL)

# 使用较高的警告级别
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()

# 源代码文件
set(SOURCES
    fingerprint_api.cpp
    fingerprint.cpp
    fingerprint_exports.cpp
    communication.cpp
    command.cpp
    device.cpp
    crypt/crypt_user.cpp
    crypt/base64.cpp
    crypt/md5.cpp
    crypt/rsa.cpp
    crypt/tea/xtea.cpp
    crypt/utils/bigint.cpp
    usbcommand.cpp
)

# 头文件
set(HEADERS
    fingerprint_api.h
    fingerprint_exports.h
    thread.h
    fingerprint.h
    communication.h
    command.h
    define.h
    device.h
    crypt/crypt_user.h
    crypt/base64.h
    crypt/md5.h
    crypt/rsa.h
    crypt/tea/xtea.h
    crypt/utils/bigint.h
    crypt/utils/types.h
    crypt/utils/utility.h
    crypt/utils/random.h
    usbcommand.h
    spti.h
)

# 创建动态链接库
add_library(fingerprint SHARED ${SOURCES} ${HEADERS})

# 设置包含目录
target_include_directories(fingerprint PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/crypt
    ${CMAKE_CURRENT_SOURCE_DIR}/crypt/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/crypt/tea
)

# 链接必要的系统库
if(UNIX AND NOT APPLE)
    target_link_libraries(fingerprint pthread)
    # 添加版本脚本
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/fingerprint.map")
elseif(WIN32)
    target_link_libraries(fingerprint setupapi)
endif()

# 安装规则
install(TARGETS fingerprint
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(FILES fingerprint_api.h fingerprint_exports.h DESTINATION include)

# 添加测试程序
add_executable(test_fingerprint test_fingerprint.cpp)
target_link_libraries(test_fingerprint fingerprint)