cmake_minimum_required(VERSION 2.8)

INCLUDE_DIRECTORIES(../include/)
ADD_EXECUTABLE(main controller.cpp homography.cpp horVanishPoint.cpp main.cpp textLocation.cpp utility.cpp)

FIND_PACKAGE(OpenCV REQUIRED)

TARGET_LINK_LIBRARIES(main ${OpenCV_LIBS})

FILE(COPY ${PROJECT_SOURCE_DIR}/rc/test.jpg	DESTINATION ${PROJECT_BINARY_DIR}/bin)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
