## set target project
file(GLOB target_src "*.h" "*.cpp")
add_executable(${subdir} ${target_src})
## set link libraries
target_link_libraries(${subdir} ${libraries})
## add local source directory to include paths
target_include_directories(${subdir} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

## copy shaders to build folder
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shader.vert DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shader.frag DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

## copy models
file(COPY ${CMAKE_SOURCE_DIR}/common/models/floor.obj DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/common/models/car_body.obj DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/common/models/car_paint.obj DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/common/models/car_spoiler.obj DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/common/models/car_wheel.obj DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/common/models/car_windows.obj DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/common/models/ant.obj DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/images/tex2b.png DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/images/tex1.png DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/images/tex3.png DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/images/tex4.png DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/images/tex2c.png DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/images/tex5.png DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
