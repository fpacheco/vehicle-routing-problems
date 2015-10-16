include_directories(
    "../"
)

## ----------------------------------------
# Append the source files here
set (TRASHC_SOURCES
     trashc.cpp
)

# Append the header files here. this will get copied to include directory
set (TRASHC_INCLUDE_FILES

)

add_executable (trashc ${TRASHC_SOURCES})
target_link_libraries (trashc LINK_PUBLIC ${VRPTOOLS_LIBRARY_NAME})

