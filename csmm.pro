TARGET = CSMM

# update this for release
VERSION = 1.0.0.0

INCLUDEPATH += /usr/local/opt/libarchive/include
LIBS += -L/usr/local/opt/libarchive/lib -larchive

QT       += core gui network concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++14

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    downloadclidialog.cpp \
    lib/addressmapping.cpp \
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
    lib/exewrapper.cpp \
    lib/fortunestreetdata.cpp \
    lib/freespacemanager.cpp \
    lib/maindol.cpp \
    lib/mapdescriptor.cpp \
    lib/patchprocess.cpp \
    lib/powerpcasm.cpp \
    lib/uimessage.cpp \
    lib/vanilladatabase.cpp \
    main.cpp \
    mainwindow.cpp \
    mapdescriptorwidget.cpp \
    venturecarddialog.cpp

HEADERS += \
    downloadclidialog.h \
    lib/addressmapping.h \
    lib/archiveutil.h \
    lib/asyncfuture.h \
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
    lib/exewrapper.h \
    lib/fortunestreetdata.h \
    lib/freespacemanager.h \
    lib/fslocale.h \
    lib/maindol.h \
    lib/mapdescriptor.h \
    lib/orderedmap.h \
    lib/patchprocess.h \
    lib/powerpcasm.h \
    lib/uimessage.h \
    lib/vanilladatabase.h \
    mainwindow.h \
    mapdescriptorwidget.h \
    venturecarddialog.h

FORMS += \
    downloadclidialog.ui \
    mainwindow.ui \
    venturecarddialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
