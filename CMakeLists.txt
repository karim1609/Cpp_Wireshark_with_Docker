cmake_minimum_required(VERSION 3.16)

project(WireScope VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

set(SOURCES
    src/main.cpp
    src/views/mainwindow.cpp
    src/viewmodels/packetsummarytablemodel.cpp
    src/services/packetcaptureservice.cpp
    src/db/databasemanager.cpp
)

set(HEADERS
    include/views/mainwindow.h
    include/viewmodels/packetsummarytablemodel.h
    include/models/core/packetsummary.h
    include/models/core/networkinterface.h
    include/services/packetcaptureservice.h
    include/db/databasemanager.h
    include/models/session/capturesession.h
)

qt_add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Widgets
        Qt6::Network
        mysqlcppconn
)
