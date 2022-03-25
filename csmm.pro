TARGET = csmm

#Application version
VERSION_MAJOR = 2
VERSION_MINOR = 0
VERSION_BUILD = 0

DEFINES += "VERSION_MAJOR=$$VERSION_MAJOR"\
       "VERSION_MINOR=$$VERSION_MINOR"\
       "VERSION_BUILD=$$VERSION_BUILD"

VERSION = $${VERSION_MAJOR}.$${VERSION_MINOR}.$${VERSION_BUILD}

QMAKE_TARGET_BUNDLE_PREFIX = com.fortunestreetmodding

QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15

INCLUDEPATH += /usr/local/opt/libarchive/include /usr/local/include lib/libbecquerel
LIBS += -L/usr/local/opt/libarchive/lib -L/usr/local/lib -L$$_PRO_FILE_PWD_/lib/libbecquerel/build -lyaml-cpp -lbecquerel
win32: INCLUDEPATH += lib/yaml-cpp/include
win32: LIBS += -L$$_PRO_FILE_PWD_/lib/yaml-cpp/build
!win32: LIBS += -larchive

QT       += core gui network concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
CONFIG += console
CONFIG += resources_big

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
DEFINES += NOMINMAX

SOURCES += \
    downloadclidialog.cpp \
    lib/addressmapping.cpp \
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
    lib/importexportutils.cpp \
    lib/mods/arc/defaultminimapicons.cpp \
    lib/mods/arc/turnlotscenes.cpp \
    lib/mods/defaultmodlist.cpp \
    lib/mods/dolio/allocatedescriptorcount.cpp \
    lib/mods/dolio/backgroundtable.cpp \
    lib/mods/dolio/bgmidtable.cpp \
    lib/mods/dolio/bgsequencetable.cpp \
    lib/mods/dolio/defaulttargetamounttable.cpp \
    lib/mods/dolio/designtypetable.cpp \
    lib/mods/dolio/displaymapinresults.cpp \
    lib/mods/dolio/dolio.cpp \
    lib/mods/dolio/doliotable.cpp \
    lib/mods/dolio/eventsquaremod.cpp \
    lib/mods/dolio/expandmapsinzone.cpp \
    lib/mods/dolio/forcesimulatedbuttonpress.cpp \
    lib/mods/dolio/frbmaptable.cpp \
    lib/mods/dolio/internalnametable.cpp \
    lib/mods/dolio/mapdescriptiontable.cpp \
    lib/mods/dolio/mapgalaxyparamtable.cpp \
    lib/mods/dolio/mapicontable.cpp \
    lib/mods/dolio/maporigintable.cpp \
    lib/mods/dolio/mapsetzoneorder.cpp \
    lib/mods/dolio/mapswitchparamtable.cpp \
    lib/mods/dolio/musictable.cpp \
    lib/mods/dolio/nameddistricts.cpp \
    lib/mods/dolio/mutatorrollshoppricemultiplier.cpp \
    lib/mods/dolio/mutatorshoppricemultiplier.cpp \
    lib/mods/dolio/mutatortable.cpp \
    lib/mods/dolio/practiceboard.cpp \
    lib/mods/dolio/rulesettable.cpp \
    lib/mods/dolio/stagenameidtable.cpp \
    lib/mods/dolio/tinydistricts.cpp \
    lib/mods/dolio/tourbankruptcylimittable.cpp \
    lib/mods/dolio/tourclearranktable.cpp \
    lib/mods/dolio/tourinitialcashtable.cpp \
    lib/mods/dolio/touropponentstable.cpp \
    lib/mods/dolio/venturecardtable.cpp \
    lib/mods/dolio/wififix.cpp \
    lib/exewrapper.cpp \
    lib/fortunestreetdata.cpp \
    lib/freespacemanager.cpp \
    lib/gameinstance.cpp \
    lib/mapdescriptor.cpp \
    lib/mods/freespace/districtnamefreespace.cpp \
    lib/mods/freespace/initialfreespace.cpp \
    lib/mods/freespace/mapdatafreespace.cpp \
    lib/mods/freespace/venturecardfreespace.cpp \
    lib/mods/freespace/wififreespace.cpp \
    lib/mods/misc/defaultmiscpatches.cpp \
    lib/mutator/mutator.cpp \
    lib/mutator/rollagain.cpp \
    lib/mutator/rollshoppricemultiplier.cpp \
    lib/mutator/shoppricemultiplier.cpp \
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
    lib/brsar.h \
    lib/bsdiff/bsdifflib.h \
    lib/bsdiff/bspatchlib.h \
    lib/bsdiff/bzip2/bzlib.h \
    lib/bsdiff/bzip2/bzlib_private.h \
    lib/configuration.h \
    lib/datafileset.h \
    lib/importexportutils.h \
    lib/mods/arc/defaultminimapicons.h \
    lib/mods/arc/turnlotscenes.h \
    lib/mods/csmmmodpack.h \
    lib/mods/defaultmodlist.h \
    lib/mods/dolio/allocatedescriptorcount.h \
    lib/mods/dolio/backgroundtable.h \
    lib/mods/dolio/bgmidtable.h \
    lib/mods/dolio/bgsequencetable.h \
    lib/mods/dolio/defaulttargetamounttable.h \
    lib/mods/dolio/designtypetable.h \
    lib/mods/dolio/displaymapinresults.h \
    lib/mods/dolio/dolio.h \
    lib/mods/dolio/doliotable.h \
    lib/mods/dolio/eventsquaremod.h \
    lib/mods/dolio/expandmapsinzone.h \
    lib/mods/dolio/forcesimulatedbuttonpress.h \
    lib/mods/dolio/frbmaptable.h \
    lib/mods/dolio/internalnametable.h \
    lib/mods/dolio/mapdescriptiontable.h \
    lib/mods/dolio/mapgalaxyparamtable.h \
    lib/mods/dolio/mapicontable.h \
    lib/mods/dolio/maporigintable.h \
    lib/mods/dolio/mapsetzoneorder.h \
    lib/mods/dolio/mapswitchparamtable.h \
    lib/mods/dolio/musictable.h \
    lib/mods/dolio/nameddistricts.h \
    lib/mods/dolio/mutatorrollshoppricemultiplier.h \
    lib/mods/dolio/mutatorshoppricemultiplier.h \
    lib/mods/dolio/mutatortable.h \
    lib/mods/dolio/practiceboard.h \
    lib/mods/dolio/rulesettable.h \
    lib/mods/dolio/stagenameidtable.h \
    lib/mods/dolio/tinydistricts.h \
    lib/mods/dolio/tourbankruptcylimittable.h \
    lib/mods/dolio/tourclearranktable.h \
    lib/mods/dolio/tourinitialcashtable.h \
    lib/mods/dolio/touropponentstable.h \
    lib/mods/dolio/venturecardtable.h \
    lib/mods/dolio/wififix.h \
    lib/downloadtools.h \
    lib/exewrapper.h \
    lib/filesystem.hpp \
    lib/fortunestreetdata.h \
    lib/freespacemanager.h \
    lib/fslocale.h \
    lib/gameinstance.h \
    lib/mapdescriptor.h \
    lib/mods/csmmmod.h \
    lib/mods/freespace/districtnamefreespace.h \
    lib/mods/freespace/initialfreespace.h \
    lib/mods/freespace/mapdatafreespace.h \
    lib/mods/freespace/venturecardfreespace.h \
    lib/mods/freespace/wififreespace.h \
    lib/mods/misc/defaultmiscpatches.h \
    lib/music.h \
    lib/mutator/mutator.h \
    lib/mutator/rollagain.h \
    lib/mutator/rollshoppricemultiplier.h \
    lib/mutator/shoppricemultiplier.h \
    lib/orderedmap.h \
    lib/powerpcasm.h \
    lib/pugixml/pugiconfig.hpp \
    lib/pugixml/pugixml.hpp \
    lib/resultscenes.h \
    lib/uigame013.h \
    lib/uigame052.h \
    lib/uimenu1900a.h \
    lib/uimessage.h \
    lib/unicodefilenameutils.h \
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

DISTFILES += \
    LICENSE \
    README.md
