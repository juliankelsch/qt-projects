cmake_minimum_required(VERSION 3.5)

project(learn-widgets VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Test)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Test)

#set(SOURCE_DIR "src")

#file(GLOB PROJECT_SOURCES
#    "*.cpp"
#    "*.h"
#    $"{SOURCE_DIR}/*.cpp"
#    $"{SOURCE_DIR}/*.h"
#)

#list(APPEND PROJECT_SOURCES resources.qrc)

set(PROJECT_SOURCES
        src/main.cpp
        src/project_hub/projecthub.cpp
        src/project_hub/projecthub.h
        src/chess/chess.h
        src/chess/chess.cpp
        resources.qrc
)


#qt_add_resources(RESOURCES resources.qrc)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(learn-widgets
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET learn-widgets APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(learn-widgets SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(learn-widgets
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(learn-widgets PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(learn-widgets PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS learn-widgets
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(learn-widgets)
endif()

# Test integration
#set(TEST_SOURCES
#    chess.h
#    chess.cpp
#    # Add more test files as needed
#)

#add_executable(learn-widgets-tests ${TEST_SOURCES})

#target_link_libraries(learn-widgets-tests PRIVATE Qt${QT_VERSION_MAJOR}::Test Qt${QT_VERSION_MAJOR}::Widgets)

#enable_testing()

#add_test(NAME LearnWidgetsTests COMMAND learn-widgets-tests)

