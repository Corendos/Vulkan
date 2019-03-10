file(
    GLOB_RECURSE
    src
    ../src/*.cpp
)

file(
    GLOB_RECURSE
    header
    ../../include/*.hpp
)

set(CURRENT_PROJECT_TEST test-project)

# Declare executable
add_executable(${CURRENT_PROJECT_TEST} test.cpp ${src})

# Include local include files
target_include_directories(${CURRENT_PROJECT_TEST} PUBLIC ${PROJECT_SOURCE_DIR}/include)

# Include Vulkan
target_include_directories(${CURRENT_PROJECT_TEST} PUBLIC ${Vulkan_INCLUDE_DIR})
target_link_libraries(${CURRENT_PROJECT_TEST} PUBLIC ${Vulkan_LIBRARIES})

# Include GLFW
target_include_directories(${CURRENT_PROJECT_TEST} PUBLIC ${glfw3_INCLUDE_DIR})
target_link_libraries(${CURRENT_PROJECT_TEST} PUBLIC glfw)

# Include GLM 
target_include_directories(${CURRENT_PROJECT_TEST} PUBLIC /usr/include/glm)

# Include Assimp
target_include_directories(${CURRENT_PROJECT_TEST} PUBLIC /usr/local/include)
target_link_libraries(${CURRENT_PROJECT_TEST} PUBLIC assimp)

# Set Debug flags
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    message("Debug option")
    target_compile_definitions(${CURRENT_PROJECT_TEST} PUBLIC DEBUG)
endif(${CMAKE_BUILD_TYPE} STREQUAL "Debug")

# Set output location
set_target_properties(${CURRENT_PROJECT_TEST}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
)