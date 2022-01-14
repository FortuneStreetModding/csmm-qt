TARGET = csmm

#Application version
VERSION_MAJOR = 1
VERSION_MINOR = 6
VERSION_BUILD = 3

DEFINES += "VERSION_MAJOR=$$VERSION_MAJOR"\
       "VERSION_MINOR=$$VERSION_MINOR"\
       "VERSION_BUILD=$$VERSION_BUILD"

VERSION = $${VERSION_MAJOR}.$${VERSION_MINOR}.$${VERSION_BUILD}

QMAKE_TARGET_BUNDLE_PREFIX = com.fortunestreetmodding

INCLUDEPATH += /usr/local/opt/libarchive/include /usr/local/include
LIBS += -L/usr/local/opt/libarchive/lib -L/usr/local/lib -lpthread
win32: INCLUDEPATH += lib/mxml lib/yaml-cpp/include
win32: LIBS += -L$$_PRO_FILE_PWD_/lib/mxml/ -L$$_PRO_FILE_PWD_/lib/yaml-cpp/build
win32: LIBS += -lmxml -lyaml-cpp
!win32: LIBS += -larchive -lmxml -lyaml-cpp

QT       += core gui network concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
CONFIG += console

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
    lib/brsar.cpp \
    lib/bsdiff/bsdifflib.c \
    lib/bsdiff/bspatchlib.c \
    lib/bsdiff/bzip2/blocksort.c \
    lib/bsdiff/bzip2/bzlib.c \
    lib/bsdiff/bzip2/compress.c \
    lib/bsdiff/bzip2/crctable.c \
    lib/bsdiff/bzip2/decompress.c \
    lib/bsdiff/bzip2/huffman.c \
    lib/bsdiff/bzip2/randtable.c \
    lib/configuration.cpp \
    lib/dolio/backgroundtable.cpp \
    lib/dolio/bgmidtable.cpp \
    lib/dolio/bgsequencetable.cpp \
    lib/dolio/defaulttargetamounttable.cpp \
    lib/dolio/designtypetable.cpp \
    lib/dolio/displaymapinresults.cpp \
    lib/dolio/dolio.cpp \
    lib/dolio/doliotable.cpp \
    lib/dolio/eventsquare.cpp \
    lib/dolio/expandmapsinzone.cpp \
    lib/dolio/forcesimulatedbuttonpress.cpp \
    lib/dolio/frbmaptable.cpp \
    lib/dolio/internalnametable.cpp \
    lib/dolio/mapdescriptiontable.cpp \
    lib/dolio/mapgalaxyparamtable.cpp \
    lib/dolio/mapicontable.cpp \
    lib/dolio/maporigintable.cpp \
    lib/dolio/mapsetzoneorder.cpp \
    lib/dolio/mapswitchparamtable.cpp \
    lib/dolio/musictable.cpp \
    lib/dolio/nameddistricts.cpp \
    lib/dolio/mutatorrollshoppricemultiplier.cpp \
    lib/dolio/mutatorshoppricemultiplier.cpp \
    lib/dolio/mutatortable.cpp \
    lib/dolio/practiceboard.cpp \
    lib/dolio/rulesettable.cpp \
    lib/dolio/stagenameidtable.cpp \
    lib/dolio/tinydistricts.cpp \
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
    lib/mutator/mutator.cpp \
    lib/mutator/rollagain.cpp \
    lib/mutator/rollshoppricemultiplier.cpp \
    lib/mutator/shoppricemultiplier.cpp \
    lib/patchprocess.cpp \
    lib/powerpcasm.cpp \
    lib/pugixml/pugixml.cpp \
    lib/resultscenes.cpp \
    lib/uigame013.cpp \
    lib/uigame052.cpp \
    lib/uimenu1900a.cpp \
    lib/uimessage.cpp \
    lib/vanilladatabase.cpp \
    lib/zip/zip.c \
    main.cpp \
    maincli.cpp \
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
    lib/await.h \
    lib/benzin/brlan.h \
    lib/benzin/brlyt.h \
    lib/benzin/endian.h \
    lib/benzin/general.h \
    lib/benzin/memfile.h \
    lib/benzin/types.h \
    lib/benzin/xml.h \
    lib/brsar.h \
    lib/bsdiff/bsdiff.h \
    lib/bsdiff/bsdifflib.h \
    lib/bsdiff/bspatch.h \
    lib/bsdiff/bspatchlib.h \
    lib/bsdiff/bzip2/bzlib.h \
    lib/bsdiff/bzip2/bzlib_private.h \
    lib/configuration.h \
    lib/datafileset.h \
    lib/dolio/backgroundtable.h \
    lib/dolio/bgmidtable.h \
    lib/dolio/bgsequencetable.h \
    lib/dolio/defaulttargetamounttable.h \
    lib/dolio/designtypetable.h \
    lib/dolio/displaymapinresults.h \
    lib/dolio/dolio.h \
    lib/dolio/doliotable.h \
    lib/dolio/eventsquare.h \
    lib/dolio/expandmapsinzone.h \
    lib/dolio/forcesimulatedbuttonpress.h \
    lib/dolio/frbmaptable.h \
    lib/dolio/internalnametable.h \
    lib/dolio/mapdescriptiontable.h \
    lib/dolio/mapgalaxyparamtable.h \
    lib/dolio/mapicontable.h \
    lib/dolio/maporigintable.h \
    lib/dolio/mapsetzoneorder.h \
    lib/dolio/mapswitchparamtable.h \
    lib/dolio/musictable.h \
    lib/dolio/nameddistricts.h \
    lib/dolio/mutatorrollshoppricemultiplier.h \
    lib/dolio/mutatorshoppricemultiplier.h \
    lib/dolio/mutatortable.h \
    lib/dolio/practiceboard.h \
    lib/dolio/rulesettable.h \
    lib/dolio/stagenameidtable.h \
    lib/dolio/tinydistricts.h \
    lib/dolio/tourbankruptcylimittable.h \
    lib/dolio/tourclearranktable.h \
    lib/dolio/tourinitialcashtable.h \
    lib/dolio/touropponentstable.h \
    lib/dolio/venturecardtable.h \
    lib/dolio/wififix.h \
    lib/downloadtools.h \
    lib/exewrapper.h \
    lib/fortunestreetdata.h \
    lib/freespacemanager.h \
    lib/fslocale.h \
    lib/maindol.h \
    lib/mapdescriptor.h \
    lib/music.h \
    lib/mutator/mutator.h \
    lib/mutator/rollagain.h \
    lib/mutator/rollshoppricemultiplier.h \
    lib/mutator/shoppricemultiplier.h \
    lib/orderedmap.h \
    lib/patchprocess.h \
    lib/powerpcasm.h \
    lib/pugixml/pugiconfig.hpp \
    lib/pugixml/pugixml.hpp \
    lib/resultscenes.h \
    lib/uigame013.h \
    lib/uigame052.h \
    lib/uimenu1900a.h \
    lib/uimessage.h \
    lib/vanilladatabase.h \
    lib/zip/miniz.h \
    lib/zip/zip.h \
    maincli.h \
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

RESOURCES += \
    csmm.qrc
