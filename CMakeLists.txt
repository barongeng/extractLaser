project(extractLaser)
#opencv
add_executable(extractLaser 
main.cpp
lsd.cpp
lsd_lines.cpp
lsd_opencv.cpp
lsd_wrap.cpp

)
 find_package(OpenCV REQUIRED)  
 target_link_libraries(extractLaser ${OpenCV_LIBS})
