cmake_minimum_required(VERSION 2.8)
project( robovid )
find_package( OpenCV REQUIRED )

add_executable( robovidc robovid.cpp )
target_link_libraries( robovidc ${OpenCV_LIBS} )

add_executable( robovidd robovidd.cpp )
target_link_libraries( robovidc ${OpenCV_LIBS} )
