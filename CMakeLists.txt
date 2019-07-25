cmake_minimum_required(VERSION 3.1.0)

set(APP_NAME "kbugscontext")

project(${APP_NAME}
        VERSION 0.1.0
        LANGUAGES CXX
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 11)

# set(CMAKE_PREFIX_PATH "/opt/Qt5.12.3/5.12.3/gcc_64")

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package(Qt5 COMPONENTS Core Gui Widgets Network REQUIRED)

set(SOURCES 
    kubeconfig.cpp
    loadconfigfromserverdialog.cpp
    main.cpp
    kbugscontext.cpp
)

set(HEADERS
    kubeconfig.h
    kbugscontext.h
    loadconfigfromserverdialog.h
)

set(RESOURCES 
    res.qrc
)

set(FORMS
    kbugscontext.ui
    loadconfigfromserverdialog.ui
)

include_directories(./include)
link_directories(./lib)

add_executable(${APP_NAME}
    ${HEADERS} ${SOURCES} ${RESOURCES} ${FORMS}
)

target_link_libraries(${APP_NAME} yaml-cpp)

target_link_libraries(${APP_NAME} Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Network)

install(TARGETS ${APP_NAME}
    RUNTIME DESTINATION bin
)

if (UNIX)
    install(FILES kbugscontext.desktop DESTINATION share/applications)
    install(FILES icons/kbugscontext.png DESTINATION share/icons/hicolor/256x256/apps)
endif()

