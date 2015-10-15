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

## ----------------------------------------
# Append the source files here
set (TIMEMATRIX_SOURCES
    ../../../../../baseClasses/basictypes.cpp
    ../../../../../baseClasses/osrmclient.cpp
    ../../../../../baseClasses/node.cpp
    ../../../../../baseClasses/stats.cpp
    ../../../../../baseClasses/vrp_assert.cpp
    timematrix.cpp
    #../../../../../baseClasses/basictypes.h
    #./../../../../baseClasses/logger.h
)

# Activa c++11
set(CMAKE_CPP_FLAGS "${CMAKE_CPP_FLAGS} ${RAPIDJSON_CXX_FLAGS}")

# Append the header files here. this will get copied to include directory
set (TIMEMATRIX_INCLUDE_FILES

)

MESSAGE(STATUS "Boost: " ${Boost_LIBRARIES} )
MESSAGE(STATUS "OSRM: " ${OSRM_LIBRARIES} )
MESSAGE(STATUS "RapidJSON: " ${RAPIDJSON_CXX_FLAGS} )
MESSAGE(STATUS "glog: " ${GLOG_LIBRARIES} )
MESSAGE(STATUS "curlpp: " ${CURLPP_LIBRARIES} )


add_executable (timematrix ${TIMEMATRIX_SOURCES})
target_link_libraries (timematrix ${OSRM_LIBRARIES} ${GLOG_LIBRARIES} ${CURLPP_LIBRARIES} ${Boost_LIBRARIES} pthread rt )
#target_link_libraries (timematrix ${OSRM_LIBRARIES} ${GLOG_LIBRARIES} ${CURLPP_LIBRARIES} ${Boost_LIBRARIES} )