cmake_minimum_required(VERSION 3.5)
project(adm_audio_renderer VERSION 1.0.0 LANGUAGES CXX)

find_package(Boost 1.57 REQUIRED)
find_package(adm REQUIRED)
find_package(ear REQUIRED)

add_executable(adm_audio_renderer src/main.cpp)
target_link_libraries(adm_audio_renderer PRIVATE ear)
target_link_libraries(adm_audio_renderer PRIVATE adm)

install(TARGETS adm_audio_renderer DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
