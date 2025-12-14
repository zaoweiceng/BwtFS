set(BINARY bwtfs_net)
file(GLOB_RECURSE NET_SOURCES LIST_DIRECTORIES false *.h *.cpp *.hpp)

set(SOURCES ${NET_SOURCES})

add_executable(${BINARY} ${NET_SOURCES})

# Include core headers
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../include")

# Include cpp-httplib and nlohmann/json headers
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../lib/include")

# Add Windows specific includes if needed
if(WIN32)
    # No additional includes needed for httplib.h and jsoncpp
endif()

add_library(${BINARY}_lib STATIC ${SOURCES})

# Link with the main BwtFS library
target_link_libraries(${BINARY} PUBLIC ${CMAKE_PROJECT_NAME}_lib)

# Platform-specific networking libraries
if(WIN32)
    target_link_libraries(${BINARY} PRIVATE Ws2_32 Mswsock Crypt32)
else()
    # Linux needs pthread for threading
    target_link_libraries(${BINARY} PRIVATE pthread)
    if (NOT APPLE)
        target_link_libraries(${BINARY} PRIVATE rt)
    endif()
endif()

# 设置输出目录
set_target_properties(${BINARY} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin  # 可执行文件输出目录
)
