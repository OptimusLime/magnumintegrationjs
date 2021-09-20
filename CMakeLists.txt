cmake_minimum_required(VERSION 3.9)
cmake_policy(SET CMP0042 NEW)
# project(MyApplication)
project (magnum-integration-js)

# Very first thing to do. Either node_modules exists within our directory
# or it exists two directories up and we are being built as a dependency (most likely)
get_filename_component(_fullpath "${CMAKE_SOURCE_DIR}/node_modules" REALPATH)
get_filename_component(_fullbackpath "${CMAKE_SOURCE_DIR}/../../node_modules" REALPATH)
get_filename_component(_fullownerpath "${CMAKE_SOURCE_DIR}/../../../node_modules" REALPATH)

message("Checking: ${_fullpath}")
message("Checking Back: ${_fullbackpath}")
message("Checking Back: ${_fullownerpath}")

IF(EXISTS ${_fullpath})
  set(BASE_NODE_DIR ${_fullpath})
elseif(EXISTS ${_fullbackpath})
  # we are a dependency for another project inside of a node_modules folder
  set(BASE_NODE_DIR ${_fullbackpath})
elseif(EXISTS ${_fullownerpath})
  # we are a dependency for another project inside of a node_modules folder
  set(BASE_NODE_DIR ${_fullownerpath})
endif()

message(STATUS "Setting Node Depends Dir: ${BASE_NODE_DIR}")

set(MAGNUMINTEGRATIONJS_MAJOR_VERSION 0)
set(MAGNUMINTEGRATIONJS_MINOR_VERSION 1)
set(MAGNUMINTEGRATIONJS_PATCH_VERSION 0)
set(MAGNUMINTEGRATIONJS_VERSION
  ${MAGNUMINTEGRATIONJS_MAJOR_VERSION}.${MAGNUMINTEGRATIONJS_MINOR_VERSION}.${MAGNUMINTEGRATIONJS_PATCH_VERSION})

  # Offer the user the choice of overriding the installation directories
set(INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries")
set(INSTALL_BIN_DIR bin CACHE PATH "Installation directory for executables")
set(INSTALL_INCLUDE_DIR include CACHE PATH
  "Installation directory for header files")

if(WIN32 AND NOT CYGWIN)
  set(DEF_INSTALL_CMAKE_DIR cmake)
else()
  set(DEF_INSTALL_CMAKE_DIR lib/cmake/MinMagApi)
endif()
set(INSTALL_CMAKE_DIR ${DEF_INSTALL_CMAKE_DIR} CACHE PATH
  "Installation directory for CMake files")

# Make relative paths absolute (needed later on)
foreach(p LIB BIN INCLUDE CMAKE)
set(var INSTALL_${p}_DIR)
if(NOT IS_ABSOLUTE "${${var}}")
  set(${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
endif()
endforeach()

set (CMAKE_CXX_STANDARD 14)
include_directories(${CMAKE_JS_INC})

# FetchContent added in CMake 3.11, downloads during the configure step
include(FetchContent)
# FetchContent_MakeAvailable was not added until CMake 3.14; use our shim
if(${CMAKE_VERSION} VERSION_LESS 3.14)
    include(cmake/add_FetchContent_MakeAvailable.cmake)
endif()

if( NOT DEFINED ${VCPKG_ROOT})
  message(STATUS "Pulling VCPKG_ROOT from Environment to build")
  set(VCPKG_ROOT $ENV{VCPKG_ROOT})
  if(NOT DEFINED ${VCPKG_ROOT})
    set(VCTOOL_FILE $ENV{VCTOOL})
    get_filename_component(VCTOOL_DIR ${VCTOOL_FILE} DIRECTORY)
    get_filename_component(VCPKG_ROOT ${VCTOOL_DIR}/../../ REALPATH)
  endif()
endif()

if( NOT DEFINED ${VCPKG_TARGET_TRIPLET})
  message(STATUS "Pulling VCPKG_TARGET_TRIPLET from Environment to build")
  set(VCPKG_TARGET_TRIPLET $ENV{VCPKG_TARGET_TRIPLET})
  if(NOT DEFINED ${VCPKG_TARGET_TRIPLET})
    set(VCPKG_TARGET_TRIPLET "x64-windows")
  endif()
endif()

add_library(${PROJECT_NAME} SHARED) 
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/modules/" ${CMAKE_MODULE_PATH})
set(CMAKE_PREFIX_PATH "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/" ${CMAKE_PREFIX_PATH})

set(EASYLOGGINGPP_ROOT ${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/include)
add_compile_definitions(ELPP_THREAD_SAFE ELPP_FORCE_USE_STD_THREAD)
add_compile_definitions(NOMINMAX)

file(GLOB EASYLOGGING ${EASYLOGGINGPP_ROOT}/*.cc)
message(STATUS "LOGGIN DIR: ${EASYLOGGINGPP_ROOT}, VCPkG: ${VCPKG_ROOT}")

find_package(fmt CONFIG REQUIRED)

set(CMAKE_MODULE_PATH 
    ${CMAKE_MODULE_PATH} 
    ${BASE_NODE_DIR}/corradejs/corrade/share/cmake/Corrade
    ${BASE_NODE_DIR}/magnumcppjs/magnum/share/cmake/Magnum)

set(CMAKE_PREFIX_PATH 
  ${CMAKE_PREFIX_PATH} 
  ${BASE_NODE_DIR}/corradejs/corrade
  ${BASE_NODE_DIR}/magnumcppjs/magnum
  )


FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG ddddabdccfdafffd8664fb4e29230dc4f848137e
)
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
  message(STATUS "Fetching imgui ${imgui_VERSION}")

  FetchContent_Populate(imgui)

  set(IMGUI_INCLUDE_DIR 
    ${imgui_SOURCE_DIR})

endif()

FetchContent_Declare(
  magnum_integration
  GIT_REPOSITORY https://github.com/mosra/magnum-integration.git
  GIT_TAG 36c1b1515873eff5047b27751c12a1580c141dd8
)
FetchContent_GetProperties(magnum_integration)
if(NOT magnum_integration_POPULATED)
    message(STATUS "Fetching MAGNUM v${MAGNUM_VERSION}")

    FetchContent_Populate(magnum_integration)

    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${magnum_integration_SOURCE_DIR}/modules)

    set(IMGUI_DIR ${IMGUI_INCLUDE_DIR})
    set(WITH_IMGUI TRUE CACHE BOOL "" FORCE)
    add_subdirectory(${magnum_integration_SOURCE_DIR} ${magnum_integration_BINARY_DIR})

    set(MAGNUM_INTEGRATION_INCLUDE_DIR 
        ${magnum_integration_SOURCE_DIR}/src
        ${magnum_integration_BUILD_DIR}/src
        )

    set(MAGNUM_INTEGRATION_LIBRARIES 
        MagnumIntegration::ImGui
        )

endif()

target_compile_definitions(
        ${PROJECT_NAME}
        PRIVATE
            # ENTT_ID_TYPE=${ENTT_ID_TYPE}
            # _ENABLE_EXTENDED_ALIGNED_STORAGE
            NOMINMAX
            # ENTT_API_EXPORT
    )

file(GLOB SRC_MAIN
    "lib/*.cpp")

target_sources(${PROJECT_NAME} PRIVATE
    ${SRC_MAIN}
    ${EASYLOGGING}
    ${EMPTY_RESOURCES}
    ${CMAKE_JS_SRC}
    )

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${CMAKE_JS_LIB} 
    ${MAGNUM_INTEGRATION_LIBRARIES}
) 

# Include Node-API wrappers
execute_process(COMMAND node -p "require('node-addon-api').include"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE NODE_ADDON_API_DIR
        )

string(REPLACE "\n" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
string(REPLACE "\"" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
string(REGEX REPLACE "[\r\n\"]" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})

target_include_directories(${PROJECT_NAME} PRIVATE 
    ./
    src
    ../external/cr
    ${CORRADE_INCLUDE_DIR}
    ${MAGNUM_INCLUDE_DIR}
    ${MAGNUM_INTEGRATION_INCLUDE_DIR}
    ${NODE_ADDON_API_DIR})

# define NPI_VERSION
add_definitions(-DNAPI_VERSION=7)
