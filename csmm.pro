TARGET = CSMM

# update this for release
VERSION = 1.0.0.0

QMAKE_TARGET_BUNDLE_PREFIX = com.fortunestreetmodding

INCLUDEPATH += /usr/local/opt/libarchive/include /usr/local/include
LIBS += -L/usr/local/opt/libarchive/lib -L/usr/local/lib -lpthread
win32: INCLUDEPATH += lib/mxml lib/yaml-cpp/include
win32: LIBS += -L$$_PRO_FILE_PWD_/lib/mxml/ -L$$_PRO_FILE_PWD_/lib/yaml-cpp/build
win32: LIBS += -lmxml -lyaml-cpp
!win32: LIBS += -larchive -lmxml -lyaml-cpp

QT       += core gui network concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++14

include(lib/qtshell/qtshell.pri)

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
DEFINES += NOMINMAX

SOURCES += \
    downloadclidialog.cpp \
    lib/addressmapping.cpp \
    lib/benzin/brlan.c \
    lib/benzin/brlyt.c \
    lib/benzin/endian.c \
    lib/benzin/general.c \
    lib/benzin/memfile.c \
    lib/benzin/xml.c \
    lib/brsar/brsar.cpp \
    lib/dolio/backgroundtable.cpp \
    lib/dolio/bgmidtable.cpp \
    lib/dolio/bgsequencetable.cpp \
    lib/dolio/defaulttargetamounttable.cpp \
    lib/dolio/designtypetable.cpp \
    lib/dolio/dolio.cpp \
    lib/dolio/doliotable.cpp \
    lib/dolio/eventsquare.cpp \
    lib/dolio/forcesimulatedbuttonpress.cpp \
    lib/dolio/frbmaptable.cpp \
    lib/dolio/internalnametable.cpp \
    lib/dolio/mapdescriptiontable.cpp \
    lib/dolio/mapgalaxyparamtable.cpp \
    lib/dolio/mapicontable.cpp \
    lib/dolio/maporigintable.cpp \
    lib/dolio/mapsetzoneorder.cpp \
    lib/dolio/mapswitchparamtable.cpp \
    lib/dolio/practiceboard.cpp \
    lib/dolio/rulesettable.cpp \
    lib/dolio/stagenameidtable.cpp \
    lib/dolio/tourbankruptcylimittable.cpp \
    lib/dolio/tourclearranktable.cpp \
    lib/dolio/tourinitialcashtable.cpp \
    lib/dolio/touropponentstable.cpp \
    lib/dolio/venturecardtable.cpp \
    lib/dolio/wififix.cpp \
    lib/exewrapper.cpp \
    lib/fortunestreetdata.cpp \
    lib/freespacemanager.cpp \
    lib/maindol.cpp \
    lib/mapdescriptor.cpp \
    lib/patchprocess.cpp \
    lib/powerpcasm.cpp \
    lib/pugixml/pugixml.cpp \
    lib/uimenu1900a.cpp \
    lib/uimessage.cpp \
    lib/vanilladatabase.cpp \
    lib/zip/zip.c \
    main.cpp \
    mainwindow.cpp \
    mapdescriptorwidget.cpp \
    validationerrordialog.cpp \
    venturecarddialog.cpp

HEADERS += \
    darkdetect.h \
    downloadclidialog.h \
    lib/addressmapping.h \
    lib/archiveutil.h \
    lib/asyncfuture.h \
    lib/benzin/brlan.h \
    lib/benzin/brlyt.h \
    lib/benzin/endian.h \
    lib/benzin/general.h \
    lib/benzin/memfile.h \
    lib/benzin/types.h \
    lib/benzin/xml.h \
    lib/brsar/brsar.h \
    lib/datafileset.h \
    lib/dolio/backgroundtable.h \
    lib/dolio/bgmidtable.h \
    lib/dolio/bgsequencetable.h \
    lib/dolio/defaulttargetamounttable.h \
    lib/dolio/designtypetable.h \
    lib/dolio/dolio.h \
    lib/dolio/doliotable.h \
    lib/dolio/eventsquare.h \
    lib/dolio/forcesimulatedbuttonpress.h \
    lib/dolio/frbmaptable.h \
    lib/dolio/internalnametable.h \
    lib/dolio/mapdescriptiontable.h \
    lib/dolio/mapgalaxyparamtable.h \
    lib/dolio/mapicontable.h \
    lib/dolio/maporigintable.h \
    lib/dolio/mapsetzoneorder.h \
    lib/dolio/mapswitchparamtable.h \
    lib/dolio/practiceboard.h \
    lib/dolio/rulesettable.h \
    lib/dolio/stagenameidtable.h \
    lib/dolio/tourbankruptcylimittable.h \
    lib/dolio/tourclearranktable.h \
    lib/dolio/tourinitialcashtable.h \
    lib/dolio/touropponentstable.h \
    lib/dolio/venturecardtable.h \
    lib/dolio/wififix.h \
    lib/exewrapper.h \
    lib/fortunestreetdata.h \
    lib/freespacemanager.h \
    lib/fslocale.h \
    lib/maindol.h \
    lib/mapdescriptor.h \
    lib/orderedmap.h \
    lib/patchprocess.h \
    lib/powerpcasm.h \
    lib/pugixml/pugiconfig.hpp \
    lib/pugixml/pugixml.hpp \
    lib/uimenu1900a.h \
    lib/uimessage.h \
    lib/vanilladatabase.h \
    lib/zip/miniz.h \
    lib/zip/zip.h \
    mainwindow.h \
    mapdescriptorwidget.h \
    validationerrordialog.h \
    venturecarddialog.h

FORMS += \
    downloadclidialog.ui \
    mainwindow.ui \
    validationerrordialog.ui \
    venturecarddialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:RC_ICONS = AppIcon.ico
macos:ICON=AppIcon.icns

DISTFILES += \
    itast.csmm.brsar
