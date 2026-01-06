cmake_minimum_required(VERSION 3.16)

project(file_upload VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Network)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appfile_upload
    main.cpp
)

qt_add_qml_module(appfile_upload
    URI file_upload
    QML_FILES
        Main.qml
    RESOURCES
        MaterialSymbolsLightAddNotesOutline.svg
        fileIco.svg
    SOURCES
        QuickFramelessWindow.cpp
        QuickFramelessWindow.h
        SOURCES TcpManager.h TcpManager.cpp
        SOURCES Controller.h Controller.cpp
        SOURCES FileWork.h FileWork.cpp
        SOURCES Constants.h
        SOURCES Singleton.h
        SOURCES SenderQueue.h SenderQueue.cpp
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appfile_upload PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appfile_upload
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE FALSE
)

target_link_libraries(appfile_upload
    PRIVATE Qt6::Quick dwmapi Qt6::Network
)

include(GNUInstallDirs)
install(TARGETS appfile_upload
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
