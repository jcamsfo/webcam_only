cmake_minimum_required(VERSION 3.0)
project(WebCamTest)

set(CMAKE_CXX_STANDARD 11)

find_package(PkgConfig REQUIRED)
pkg_check_modules(AVFORMAT libavformat REQUIRED)
pkg_check_modules(AVCODEC libavcodec REQUIRED)
pkg_check_modules(AVUTIL libavutil REQUIRED)
pkg_check_modules(SWSCALE libswscale REQUIRED)
find_package(OpenCV REQUIRED)

include_directories(${AVFORMAT_INCLUDE_DIRS}
                    ${AVCODEC_INCLUDE_DIRS}
                    ${AVUTIL_INCLUDE_DIRS}
                    ${SWSCALE_INCLUDE_DIRS}
                    ${OpenCV_INCLUDE_DIRS})

add_executable(WebCamTest main.cpp camera_grab.cpp)

target_link_libraries(WebCamTest ${AVFORMAT_LIBRARIES}
                      ${AVCODEC_LIBRARIES}
                      ${AVUTIL_LIBRARIES}
                      ${SWSCALE_LIBRARIES}
                      ${OpenCV_LIBS})


# Link pthread library
target_link_libraries(WebCamTest pthread)                    