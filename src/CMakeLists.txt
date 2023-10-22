qt_add_executable(azharja
    data.cpp
    datasavethread.cpp
    item.cpp
    itemexplorer.cpp
    itemparentswidget.cpp
    itemwidget.cpp
    main.cpp
    mainwindow.cpp
    qtlocalpeer.cpp
    qtsingleapplication.cpp
    qtsinglecoreapplication.cpp
    scintillaeditcustom.cpp
)

target_link_libraries(azharja PRIVATE Qt6::Widgets Qt6::Core Qt6::Gui Qt6::Network Qt6::Core5Compat ScintillaEditBase ScintillaEdit)

set_target_properties(azharja PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)