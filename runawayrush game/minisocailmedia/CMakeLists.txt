cmake_minimum_required(VERSION 3.16)

project(social_media_daa VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql)

# ⭐ Put ALL source files here!
set(PROJECT_SOURCES
    main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        loginwindow.cpp
        loginwindow.h
        loginwindow.ui
        database.h
        database.cpp
        graph.h
        graph.cpp
        user.h
        user.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(social_media_daa
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}


        messagingwidget.h messagingwidget.cpp messagingwidget.ui
        postwidget.h postwidget.cpp postwidget.ui
        connectionwidget.h connectionwidget.cpp connectionwidget.ui
        loginwindow.h loginwindow.cpp loginwindow.ui
        resource.qrc


    )
else()
    if(ANDROID)
        add_library(social_media_daa SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(social_media_daa
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(social_media_daa PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
)

if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.social_media_daa)
endif()

set_target_properties(social_media_daa PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS social_media_daa
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(social_media_daa)
endif()
