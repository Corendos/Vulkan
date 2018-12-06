file(
    GLOB_RECURSE
    src
    *.cpp
)

file(
    GLOB
    headers
    *.hpp
)

set(CURRENT_PROJECT ${CMAKE_PROJECT_NAME}-${CMAKE_BUILD_TYPE})

# Declare executable
add_executable(${CURRENT_PROJECT} ${src})

# Include local include files
target_include_directories(${CURRENT_PROJECT} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)

# Include Vulkan
target_include_directories(${CURRENT_PROJECT} PUBLIC ${Vulkan_INCLUDE_DIR})
target_link_libraries(${CURRENT_PROJECT} PUBLIC ${Vulkan_LIBRARIES})

# Include GLFW
target_include_directories(${CURRENT_PROJECT} PUBLIC ${glfw3_INCLUDE_DIR})
target_link_libraries(${CURRENT_PROJECT} PUBLIC glfw)

# Include GLM 
target_include_directories(${CURRENT_PROJECT} PUBLIC /usr/include/glm)

# Include Assimp
target_include_directories(${CURRENT_PROJECT} PUBLIC /usr/local/include)
target_link_libraries(${CURRENT_PROJECT} PUBLIC assimp)

# Set Debug flags
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    message("Debug option")
    target_compile_definitions(${CURRENT_PROJECT} PUBLIC DEBUG)
endif(${CMAKE_BUILD_TYPE} STREQUAL "Debug")

# Set output location
set_target_properties(${CURRENT_PROJECT}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin"
)