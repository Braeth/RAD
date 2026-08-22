cmake_minimum_required(VERSION 3.22)


if(WIN32 AND MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

project(rad C)

if(WIN32)
    set(YAML_LIB_REF yaml.lib)
else()
    set(YAML_LIB_REF libyaml.a)
endif()

add_library(yaml STATIC IMPORTED)

set_target_properties(yaml PROPERTIES
    IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/lib/libyaml/${YAML_LIB_REF}"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/lib/libyaml/include"
)

if(WIN32)
    target_compile_definitions(yaml INTERFACE YAML_DECLARE_STATIC)
endif()


if(WIN32)
    add_library(wingetopt STATIC IMPORTED)
    set_target_properties(wingetopt PROPERTIES
        IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/lib/wingetopt/wingetopt.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/lib/wingetopt/include"
    )
endif()

set(RAD_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rad.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cli.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rad_parser.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ansi.c
)

add_executable(rad ${RAD_SOURCE})

target_include_directories(rad PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/includes)

target_link_libraries(rad PRIVATE yaml)

if(WIN32)
    target_link_libraries(rad PRIVATE wingetopt)
    target_compile_definitions(rad PRIVATE
        _CRT_SECURE_NO_WARNINGS
        _CRT_NONSTDC_NO_DEPRECATE
        popen=_popen
        pclose=_pclose
    )
endif()


message("Done!!!")