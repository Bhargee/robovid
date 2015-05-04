cmake_minimum_required(VERSION 2.8)
project( robovid )
find_package( OpenCV REQUIRED )

add_executable( robovidc robovid.cpp )
target_link_libraries( robovidc ${OpenCV_LIBS} )
target_link_libraries( robovidc "/usr/local/lib/liblcm.so.1.3.3" )
#set(CMAKE_SHARED_LINKER_FLAGS "-llcm")

add_executable( robovidd robovidd.cpp )
target_link_libraries( robovidd ${OpenCV_LIBS} )
