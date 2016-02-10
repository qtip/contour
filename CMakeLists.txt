cmake_minimum_required (VERSION 3.1.0)
#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)


project(contour)
find_package (Threads REQUIRED)
find_package (PNG)

if (PNG_FOUND)
    add_executable(contour_png 
        contour.cpp
        contour_png.cpp
        PNGImage.cpp
    )
    set_property(TARGET contour_png PROPERTY CXX_STANDARD 14)
    target_include_directories(contour_png PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries (contour_png ${CMAKE_THREAD_LIBS_INIT})

    target_link_libraries (contour_png ${PNG_LIBRARIES})
    target_include_directories(contour_png PUBLIC ${PNG_INCLUDE_DIRS})
endif (PNG_FOUND)


add_executable(contour_flt 
    contour.cpp
    contour_flt.cpp
)
set_property(TARGET contour_flt PROPERTY CXX_STANDARD 14)
target_link_libraries (contour_flt ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(contour_flt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
