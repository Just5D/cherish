if(POLICY CMP0020)
    CMAKE_POLICY(SET CMP0020 NEW)
endif()

include_directories(
    ${CMAKE_SOURCE_DIR}/cherish
    ${CMAKE_SOURCE_DIR}/libSGEntities
    ${CMAKE_SOURCE_DIR}/libSGControls
)

set(libGUI_SRCS
    CherishApplication.cpp
    CherishApplication.h
    Actions.h
    Actions.cpp
    MainWindow.cpp
    MainWindow.h
    #CMainWindow.h
    #CMainWindow.cpp
    GLWidget.h
    GLWidget.cpp
    ListWidget.h
    ListWidget.cpp
    TreeWidget.h
    TreeWidget.cpp
    ListDelegate.h
    ListDelegate.cpp
    ListView.h
    ListView.cpp
    CameraProperties.h
    CameraProperties.cpp
    PhotoModel.h
    PhotoModel.cpp
)

qt5_wrap_ui(UI_GENERATED_SRCS
    Widgets/CameraProperties.ui
    #Widgets/messageform.ui
    #Widgets/CMainWindow.ui
)

add_library(libGUI STATIC
    ${libGUI_SRCS}
    ${UI_GENERATED_SRCS}
)

target_link_libraries( libGUI
    ${QT_LIBRARIES}
    ${OPENSCENEGRAPH_LIBRARIES}
)
