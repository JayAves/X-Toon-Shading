# obtain the list of subdirectories
SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_LIST_DIR})

# ---------------------------------------------------------------------------------
# Executable and target include/link libraries
# ---------------------------------------------------------------------------------
# list of libraries
set(libraries glad glfw SOIL2)

if(APPLE)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(OPENGL_LIBRARY OpenGL)
    find_library(COREVIDEO_LIBRARY CoreVideo)

    list(APPEND libraries
            ${OPENGL_LIBRARY}
            ${COCOA_LIBRARY}
            ${IOKIT_LIBRARY}
            ${COREVIDEO_LIBRARY}
            )
endif()


FOREACH(subdir ${SUBDIRS})
    add_subdirectory(${subdir})
ENDFOREACH()

## copy shaders to build folder
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/tex2.png DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/tex3.png DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/tex1.png DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/tex4.png DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

