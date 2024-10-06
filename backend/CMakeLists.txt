cmake_minimum_required(VERSION 2.6)
project(weiss_gripkit)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "-std=c++14 -fPIC ${CMAKE_CXX_FLAGS}")
if(CMAKE_BUILD_TYPE STREQUAL "Debug") 
    set(CMAKE_CXX_FLAGS "-g ${CMAKE_CXX_FLAGS}")
endif()
set(KR2_API_LIBS kr2rc_api kr2_io_api kr2_program_api)

link_directories(/usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib $ENV{KR2_INSTALL_FOLDER}/lib $ENV{CBUN_INSTALL_FOLDER}/lib)

include_directories(include
                    /opt/kr2/include
                    ${Boost_INCLUDE_DIRS})

# Provide all your CBun source files (*.cpp)
add_library(${PROJECT_NAME} SHARED
            src/periodic_thread.cpp
            src/gripkit_cr_easy.cpp
)
target_link_libraries(${PROJECT_NAME} ${KR2_API_LIBS} ${Boost_LIBRARIES})
install(TARGETS ${PROJECT_NAME} DESTINATION $ENV{CBUN_INSTALL_FOLDER}/lib)

# Build CBun from bundle.xml, headers and CBun lib
set(CBUN_BUILD_DIR ${PROJECT_NAME}.cbunbuild)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND rm -rf ${CBUN_BUILD_DIR}
                COMMAND mkdir ${CBUN_BUILD_DIR}
                COMMAND cp lib${PROJECT_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX} ${CBUN_BUILD_DIR}
                COMMAND ${PROJECT_SOURCE_DIR}/../frontend/gradlew -p ${PROJECT_SOURCE_DIR}/../frontend clean
                COMMAND ${PROJECT_SOURCE_DIR}/../frontend/gradlew -p ${PROJECT_SOURCE_DIR}/../frontend assembleRelease
                COMMAND cp ${PROJECT_SOURCE_DIR}/../frontend/app/build/outputs/apk/release/app-release-unsigned.apk ${CBUN_BUILD_DIR}/weiss_robotics_gripkit.apk
                COMMAND cp ${PROJECT_SOURCE_DIR}/../bundle.xml ${CBUN_BUILD_DIR}
                COMMAND cp -r ${PROJECT_SOURCE_DIR}/include ${CBUN_BUILD_DIR}
                COMMAND tar -cvzf ${PROJECT_NAME}.cbun -C ${CBUN_BUILD_DIR} .
                )
