cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Build tests" ON)
option(BUILD_DOCS "Build documentation" OFF)
option(BUILD_COVERAGE "Build code coverage" OFF)

set(
  HUNTER_CACHE_SERVERS
  "https://github.com/bmstu-iu8-cpp-sem-3/hunter-cache"
  CACHE STRING "Default cache server"
)

include("cmake/HunterGate.cmake")

HunterGate(
  URL "https://github.com/cpp-pm/hunter/archive/v0.23.278.tar.gz"
  SHA1 "35fa55bc1dfcf9f4d7264f8bb26cd1759ce1ae07"
)

# TODO: rename project and delete this comment
project(builder VERSION 0.1.0)
string(APPEND CMAKE_CXX_FLAGS " -pedantic -Werror -Wall -Wextra")
string(APPEND CMAKE_CXX_FLAGS " -Wno-unused-command-line-argument")
string(APPEND CMAKE_CXX_FLAGS " -Wshadow -Wnon-virtual-dtor")

hunter_add_package(GTest)
find_package(GTest CONFIG REQUIRED)

#set(Boost_INCLUDE_DIR /home/lamp/boost_1_75_0)
hunter_add_package(Boost COMPONENTS system filesystem program_options thread log timer)
find_package(Boost COMPONENTS system filesystem program_options thread log log_setup timer)

hunter_add_package(Async++)
find_package(Async++ CONFIG REQUIRED)

hunter_add_package(nlohmann_json)
find_package(nlohmann_json CONFIG REQUIRED)


find_package(Threads)

set(LIBS Boost::boost
${Boost_LOG_SETUP_LIBRARY}
${Boost_LOG_LIBRARY}
${Boost_FILESYSTEM_LIBRARY}
${Boost_THREAD_LIBRARY}
${Boost_SYSTEM_LIBRARY}
Boost::timer Boost::program_options
nlohmann_json::nlohmann_json
Async++::Async++
)



#=================================== BUILD LIBRARY ===================================
add_library(${PROJECT_NAME}_lib STATIC
  # enum your files and delete this comment
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/builder.cpp
)

target_include_directories(${PROJECT_NAME}_lib PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>"
  "$<INSTALL_INTERFACE:include/>"
)

target_link_libraries(${PROJECT_NAME}_lib pthread ${LIBS} )

#=================================== BUILD LIBRARY ===================================

#=================================== BUILD EXECUTABLE ================================

add_executable(${PROJECT_NAME}
  ${CMAKE_CURRENT_SOURCE_DIR}/demo/main.cpp
)

target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_lib pthread ${LIBS})

#=================================== BUILD EXECUTABLE ================================

include(CPackConfig.cmake)

install(TARGETS ${PROJECT_NAME}_lib ${PROJECT_NAME} 
  EXPORT "${PROJECT_NAME}-targets"
  RUNTIME DESTINATION bin
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  INCLUDES DESTINATION include
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include)

install(EXPORT "${PROJECT_NAME}-targets"
  NAMESPACE "${PROJECT_NAME}::"
  DESTINATION "lib/cmake"
)
