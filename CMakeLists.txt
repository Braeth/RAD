cmake_minimum_required(VERSION 3.22)

project(rad C)

include(FetchContent)


FetchContent_Declare(libyaml
	GIT_REPOSITORY https://github.com/yaml/libyaml.git
	GIT_TAG 0.2.5
)

message("Downloading and installing required dependencies...")
FetchContent_MakeAvailable(libyaml)


include_directories(includes)

set(RAD_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rad.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cli.c
)

add_executable(rad ${RAD_SOURCE})

target_link_libraries(rad yaml)

target_link_options(rad PRIVATE -static)

message("Done!!!")
