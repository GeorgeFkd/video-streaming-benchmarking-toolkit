cmake_minimum_required(VERSION 3.22)

project(precise_timer VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(PkgConfig REQUIRED)
# pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
# pkg_check_modules(GSTREAMER_WEBRTC REQUIRED gstreamer-webrtc-1.0)
# pkg_check_modules(GSTREAMER_SDP REQUIRED gstreamer-sdp-1.0)
# pkg_check_modules(GSTREAMER_VIDEO REQUIRED gstreamer-video-1.0)
# pkg_check_modules(CAIRO REQUIRED cairo)
# pkg_check_modules(GTK REQUIRED gtk+-3.0)
#
# if(NOT GSTREAMER_FOUND)
#     message(FATAL_ERROR "GStreamer not found!")
# endif()
# if(NOT GSTREAMER_WEBRTC_FOUND)
#     message(FATAL_ERROR "GStreamer WebRTC not found!")
# endif()
# if(NOT GSTREAMER_SDP_FOUND)
#     message(FATAL_ERROR "GStreamer SDP not found!")
# endif()
# if(NOT CAIRO_FOUND)
#     message(FATAL_ERROR "Cairo not found!")
# endif()
# if(NOT GTK_FOUND)
#     message(FATAL_ERROR "GTK not found! Install with: sudo apt install libgtk-3-dev")
# endif()
# if (NOT GSTREAMER_VIDEO_FOUND)
# message(FATAL_ERROR "GStreamer video was not found")
# endif()
#
#
# message(STATUS "GStreamer include dirs: ${GSTREAMER_INCLUDE_DIRS}")
# message(STATUS "Cairo include dirs: ${CAIRO_INCLUDE_DIRS}")
# message(STATUS "GTK include dirs: ${GTK_INCLUDE_DIRS}")
# include_directories(
#     ${GSTREAMER_INCLUDE_DIRS}
#     ${CAIRO_INCLUDE_DIRS}
#     ${GTK_INCLUDE_DIRS}
#     ${GSTREAMER_VIDEO_INCLUDE_DIRS}
# )
#
#
#
# # ---------------------------------------------------------------------
# # Link directories
# # ---------------------------------------------------------------------
# link_directories(
#     ${GSTREAMER_LIBRARY_DIRS}
#     ${CAIRO_LIBRARY_DIRS}
#     ${GTK_LIBRARY_DIRS}
#     ${GSTREAMER_VIDEO_LIBRARY_DIRS}
# )
# qt6_dir=~/Qt/6.7.2/gcc_64
find_package(Qt6 REQUIRED COMPONENTS Gui Widgets Core Multimedia )
qt_standard_project_setup()

qt_add_executable(precise_timer 
	main.cpp
	PreciseTimer.h
	PreciseTimer.cpp
)

# qt_add_qml_module(precise_timer
# 	URI PreciseTimer2
# 	QML_FILES PreciseTimer2.qml

# )

target_link_libraries(precise_timer PRIVATE  Qt6::Gui Qt6::Core Qt6::Widgets Qt6::Multimedia)
# target_link_libraries(precise_timer PRIVATE 
#     ${GSTREAMER_LIBRARIES}
#     ${GSTREAMER_WEBRTC_LIBRARIES}
#     ${GSTREAMER_SDP_LIBRARIES}
#     ${GSTREAMER_VIDEO_LIBRARIES}
# )
