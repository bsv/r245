QT = gui \
    core \
    xml
CONFIG += qt \
    warn_on \
    release\
    static
DESTDIR = bin
OBJECTS_DIR = build
MOC_DIR = build
UI_DIR = build
FORMS = ui/update_addr_dialog.ui \
    ui/settings_window.ui \
    ui/main_window.ui \
    ui/monitor_window.ui
HEADERS = src/update_addr_dialog.h \
    src/settings_window.h \
    src/main_window.h \
    src/utils.h \
    src/monitor_window.h \
    src/r245_types.h \
    src/global.h \
    src/settings_obj.h \
    src/monitor.h \
    src/event_delegate.h \
    src/monitor_filter.h \
    src/dev_model.h
SOURCES = src/update_addr_dialog.cpp \
    src/main.cpp \
    src/settings_window.cpp \
    src/main_window.cpp \
    src/utils.cpp \
    src/monitor_window.cpp \
    src/settings_obj.cpp \
    src/monitor.cpp \
    src/event_delegate.cpp \
    src/monitor_filter.cpp \
    src/dev_model.cpp
TEMPLATE = app
