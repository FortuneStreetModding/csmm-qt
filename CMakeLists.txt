cmake_minimum_required(VERSION 3.21.1)

add_compile_definitions(QT_NO_KEYWORDS)
add_compile_definitions(UNICODE)
add_compile_definitions(_UNICODE)

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15)
    set(CMAKE_OSX_ARCHITECTURES "x86_64")
endif()

if(WIN32)
    add_definitions(/wd4996)
    add_definitions(/wd4267)
endif()

set(PROJECT_VERSION 7.3.0)

project(csmm VERSION ${PROJECT_VERSION} LANGUAGES CXX C)


set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Concurrent Core Gui Network Widgets LinguistTools REQUIRED)

qt_standard_project_setup(
    I18N_SOURCE_LANGUAGE en_US
    I18N_TRANSLATED_LANGUAGES it_IT ja_JP
)

qt_add_translations(csmm
    TS_FILES
        languages/en_US.ts
        languages/it_IT.ts
        languages/ja_JP.ts
    RESOURCE_PREFIX "translations"
)

set(LIB_SOURCES
    lib/addressmapping.cpp
    lib/addressmapping.h
    lib/asyncfuture/asyncfuture.h
    lib/await.h
    lib/brsar.cpp
    lib/brsar.h
    lib/bsdiff/bsdifflib.c
    lib/bsdiff/bsdifflib.h
    lib/bsdiff/bspatchlib.c
    lib/bsdiff/bspatchlib.h
    lib/bsdiff/bzip2/blocksort.c
    lib/bsdiff/bzip2/bzlib_private.h
    lib/bsdiff/bzip2/bzlib.c
    lib/bsdiff/bzip2/bzlib.h
    lib/bsdiff/bzip2/compress.c
    lib/bsdiff/bzip2/crctable.c
    lib/bsdiff/bzip2/decompress.c
    lib/bsdiff/bzip2/huffman.c
    lib/bsdiff/bzip2/randtable.c
    lib/configuration.cpp
    lib/configuration.h
    lib/datafileset.h
    lib/exewrapper.cpp
    lib/exewrapper.h
    lib/fortunestreetdata.cpp
    lib/fortunestreetdata.h
    lib/freespacemanager.cpp
    lib/freespacemanager.h
    lib/fslocale.h
    lib/gameinstance.cpp
    lib/gameinstance.h
    lib/importexportutils.cpp
    lib/importexportutils.h
    lib/mapdescriptor.cpp
    lib/mapdescriptor.h
    lib/mods/arc/defaultminimapicons.cpp
    lib/mods/arc/defaultminimapicons.h
    lib/mods/arc/turnlotscenes.cpp
    lib/mods/arc/turnlotscenes.h
    lib/mods/csmmmod.h
    lib/mods/csmmmodpack.h
    lib/mods/defaultmodlist.cpp
    lib/mods/defaultmodlist.h
    lib/mods/dolio/allocatedescriptorcount.cpp
    lib/mods/dolio/allocatedescriptorcount.h
    lib/mods/dolio/backgroundtable.cpp
    lib/mods/dolio/backgroundtable.h
    lib/mods/dolio/bgmidtable.cpp
    lib/mods/dolio/bgmidtable.h
    lib/mods/dolio/bgsequencetable.cpp
    lib/mods/dolio/bgsequencetable.h
    lib/mods/dolio/defaulttargetamounttable.cpp
    lib/mods/dolio/defaulttargetamounttable.h
    lib/mods/dolio/designtypetable.cpp
    lib/mods/dolio/designtypetable.h
    lib/mods/dolio/displaymapinresults.cpp
    lib/mods/dolio/displaymapinresults.h
    lib/mods/dolio/dolio.cpp
    lib/mods/dolio/dolio.h
    lib/mods/dolio/doliotable.cpp
    lib/mods/dolio/doliotable.h
    lib/mods/dolio/eventsquaremod.cpp
    lib/mods/dolio/eventsquaremod.h
    lib/mods/dolio/expandmapsinzone.cpp
    lib/mods/dolio/expandmapsinzone.h
    lib/mods/dolio/forcesimulatedbuttonpress.cpp
    lib/mods/dolio/forcesimulatedbuttonpress.h
    lib/mods/dolio/frbmaptable.cpp
    lib/mods/dolio/frbmaptable.h
    lib/mods/dolio/internalnametable.cpp
    lib/mods/dolio/internalnametable.h
    lib/mods/dolio/mapdescriptiontable.cpp
    lib/mods/dolio/mapdescriptiontable.h
    lib/mods/dolio/mapgalaxyparamtable.cpp
    lib/mods/dolio/mapgalaxyparamtable.h
    lib/mods/dolio/mapicontable.cpp
    lib/mods/dolio/mapicontable.h
    lib/mods/dolio/maporigintable.cpp
    lib/mods/dolio/maporigintable.h
    lib/mods/dolio/mapsetzoneorder.cpp
    lib/mods/dolio/mapsetzoneorder.h
    lib/mods/dolio/mapswitchparamtable.cpp
    lib/mods/dolio/mapswitchparamtable.h
    lib/mods/dolio/musictable.cpp
    lib/mods/dolio/musictable.h
    lib/mods/dolio/mutatorrollshoppricemultiplier.cpp
    lib/mods/dolio/mutatorrollshoppricemultiplier.h
    lib/mods/dolio/mutatorshopprice.cpp
    lib/mods/dolio/mutatorshopprice.h
    lib/mods/dolio/mutatorstockprice.cpp
    lib/mods/dolio/mutatorstockprice.h
    lib/mods/dolio/mutatortable.cpp
    lib/mods/dolio/mutatortable.h
    lib/mods/dolio/nameddistricts.cpp
    lib/mods/dolio/nameddistricts.h
    lib/mods/dolio/practiceboard.cpp
    lib/mods/dolio/practiceboard.h
    lib/mods/dolio/rulesettable.cpp
    lib/mods/dolio/rulesettable.h
    lib/mods/dolio/stagenameidtable.cpp
    lib/mods/dolio/stagenameidtable.h
    lib/mods/dolio/tinydistricts.cpp
    lib/mods/dolio/tinydistricts.h
    lib/mods/dolio/tourbankruptcylimittable.cpp
    lib/mods/dolio/tourbankruptcylimittable.h
    lib/mods/dolio/tourclearranktable.cpp
    lib/mods/dolio/tourclearranktable.h
    lib/mods/dolio/tourinitialcashtable.cpp
    lib/mods/dolio/tourinitialcashtable.h
    lib/mods/dolio/touropponentstable.cpp
    lib/mods/dolio/touropponentstable.h
    lib/mods/dolio/venturecardtable.cpp
    lib/mods/dolio/venturecardtable.h
    lib/mods/dolio/wififix.cpp
    lib/mods/dolio/wififix.h
    lib/mods/freespace/districtnamefreespace.cpp
    lib/mods/freespace/districtnamefreespace.h
    lib/mods/freespace/initialfreespace.cpp
    lib/mods/freespace/initialfreespace.h
    lib/mods/freespace/mapdatafreespace.cpp
    lib/mods/freespace/mapdatafreespace.h
    lib/mods/freespace/venturecardfreespace.cpp
    lib/mods/freespace/venturecardfreespace.h
    lib/mods/freespace/wififreespace.cpp
    lib/mods/freespace/wififreespace.h
    lib/mods/freespace/lowercaseshopnamefreespace.cpp
    lib/mods/freespace/lowercaseshopnamefreespace.h
    lib/mods/misc/defaultmiscpatches.cpp
    lib/mods/misc/defaultmiscpatches.h
    lib/music.h
    lib/mutator/mutator.cpp
    lib/mutator/mutator.h
    lib/mutator/rollshoppricemultiplier.cpp
    lib/mutator/rollshoppricemultiplier.h
    lib/mutator/shopprice.cpp
    lib/mutator/shopprice.h
    lib/mutator/stockprice.cpp
    lib/mutator/stockprice.h
    lib/orderedmap.h
    lib/powerpcasm.cpp
    lib/powerpcasm.h
    lib/region.cpp
    lib/region.h
    lib/resultscenes.cpp
    lib/resultscenes.h
    lib/uigame013.cpp
    lib/uigame013.h
    lib/uigame052.cpp
    lib/uigame052.h
    lib/uimenu1900a.cpp
    lib/uimenu1900a.h
    lib/uimessage.cpp
    lib/uimessage.h
    lib/vanilladatabase.cpp
    lib/vanilladatabase.h
    lib/zip/miniz.h
    lib/zip/zip.c
    lib/zip/zip.h
    lib/python/pythonbindings.h lib/python/pythonbindings.cpp
    lib/mods/modloader.h lib/mods/modloader.cpp
    lib/mods/csmmmod_decl.h
    lib/getordefault.h
    lib/mods/dolio/doliomutator.h lib/mods/dolio/doliomutator.cpp
    lib/mods/freespace/tinydistrictsfreespace.h lib/mods/freespace/tinydistrictsfreespace.cpp
    lib/python/pyobjcopywrapper.h
    lib/mods/dolio/arbitrarynumswitchstates.h lib/mods/dolio/arbitrarynumswitchstates.cpp
    lib/mods/misc/readfrbfileinfo.h lib/mods/misc/readfrbfileinfo.cpp
    lib/csmmnetworkmanager.h lib/csmmnetworkmanager.cpp
    lib/mods/dolio/customshopnames.h lib/mods/dolio/customshopnames.cpp
    lib/riivolution.h lib/riivolution.cpp
    lib/mods/misc/copymapfiles.h lib/mods/misc/copymapfiles.cpp
    lib/mods/dolio/changemusiconswitch.h lib/mods/dolio/changemusiconswitch.cpp
    lib/mods/freespace/vmovestopfreespace.h lib/mods/freespace/vmovestopfreespace.cpp
    lib/progresscanceled.h
)

qt_add_big_resources(LIB_SOURCES csmm.qrc)

set(MAIN_SOURCES
    ${LIB_SOURCES}
    additionalmodsdialog.h
    additionalmodsdialog.cpp
    additionalmodsdialog.ui
    choosemode.h
    choosemode.cpp
    choosemode.ui
    csmmmode.h
    csmmprogressdialog.h
    csmmprogressdialog.cpp
    darkdetect.h
    main.cpp
    maincli.cpp
    maincli.h
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    mapdescriptorwidget.cpp
    mapdescriptorwidget.h
    preferencesdialog.cpp
    preferencesdialog.h
    preferencesdialog.ui
    quicksetupdialog.cpp
    quicksetupdialog.h
    quicksetupdialog.ui
    usersettings.cpp
    usersettings.h
    validationerrordialog.cpp
    validationerrordialog.h
    validationerrordialog.ui
    venturecarddialog.cpp
    venturecarddialog.h
    venturecarddialog.ui
)

set(PY_SOURCES
    ${LIB_SOURCES}
    pymain.cpp
)

if(APPLE)
    set(Python3_FIND_VIRTUALENV ONLY)
endif()

find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

# copy over python libraries
if(APPLE)
    set(PYTHON_STDLIB_DESTFOLDER "${CMAKE_CURRENT_BINARY_DIR}/lib")
elseif(WIN32)
    set(PYTHON_STDLIB_DESTFOLDER "${CMAKE_CURRENT_BINARY_DIR}/py")
else()
    set(PYTHON_STDLIB_DESTFOLDER "${CMAKE_CURRENT_BINARY_DIR}/py/lib")
endif()

file(MAKE_DIRECTORY ${PYTHON_STDLIB_DESTFOLDER})
file(COPY "${Python3_STDLIB}" DESTINATION ${PYTHON_STDLIB_DESTFOLDER}
    PATTERN "config-*/*" EXCLUDE
    PATTERN ".DS_Store" EXCLUDE
    PATTERN "site-packages" EXCLUDE
    PATTERN "test" EXCLUDE
    PATTERN "__pycache__" EXCLUDE)

if(APPLE)
    set(MACOSX_BUNDLE_ICON_FILE AppIcon.icns)
    set(APP_ICON_MACOSX ${CMAKE_CURRENT_SOURCE_DIR}/AppIcon.icns)
    set_source_files_properties(${APP_ICON_MACOSX} PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources")
    # copy python library files to bundle
    file(GLOB_RECURSE PY_STDLIB_FILES "${PYTHON_STDLIB_DESTFOLDER}/python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}/*")
    foreach (PY_STDLIB_FILE ${PY_STDLIB_FILES})
        file(RELATIVE_PATH PY_STDLIB_FILE_REL "${PYTHON_STDLIB_DESTFOLDER}" ${PY_STDLIB_FILE})
        get_filename_component(PY_STDLIB_FILE_REL_DIR ${PY_STDLIB_FILE_REL} DIRECTORY)
        set_property(SOURCE ${PY_STDLIB_FILE} PROPERTY MACOSX_PACKAGE_LOCATION "Resources/lib/${PY_STDLIB_FILE_REL_DIR}")
    endforeach()
    add_executable(csmm MACOSX_BUNDLE ${MAIN_SOURCES} ${APP_ICON_MACOSX} ${PY_STDLIB_FILES} ${QM_FILES})
    add_executable(csmmpython ${PY_SOURCES} ${QM_FILES})

    file(DOWNLOAD https://wit.wiimm.de/download/wit-v3.05a-r8638-mac.tar.gz ${CMAKE_CURRENT_BINARY_DIR}/wit.tar.gz)
    add_custom_command(TARGET csmm POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E tar xzf ${CMAKE_CURRENT_BINARY_DIR}/wit.tar.gz
        COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_CURRENT_BINARY_DIR}/csmm.app/Contents/MacOS/wit
        COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_CURRENT_BINARY_DIR}/wit-v3.05a-r8638-mac ${CMAKE_CURRENT_BINARY_DIR}/csmm.app/Contents/MacOS/wit
    )
    file(DOWNLOAD https://szs.wiimm.de/download/szs-v2.40b-r8923-mac64.tar.gz ${CMAKE_CURRENT_BINARY_DIR}/szs.tar.gz)
    add_custom_command(TARGET csmm POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E tar xzf ${CMAKE_CURRENT_BINARY_DIR}/szs.tar.gz
        COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_CURRENT_BINARY_DIR}/csmm.app/Contents/MacOS/szs
        COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_CURRENT_BINARY_DIR}/szs-v2.40b-r8923-mac64 ${CMAKE_CURRENT_BINARY_DIR}/csmm.app/Contents/MacOS/szs
    )
    add_custom_command(TARGET csmmpython POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_CURRENT_BINARY_DIR}/csmmpython ${CMAKE_CURRENT_BINARY_DIR}/csmm.app/Contents/MacOS/csmmpython
    )
elseif(WIN32)
    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/AppIcon.rc")
    add_executable(csmm ${MAIN_SOURCES} ${APP_ICON_RESOURCE_WINDOWS} ${QM_FILES})
    add_executable(csmmpython ${PY_SOURCES} ${QM_FILES})

    file(DOWNLOAD https://wit.wiimm.de/download/wit-v3.05a-r8638-cygwin64.zip ${CMAKE_CURRENT_BINARY_DIR}/wit.zip)
    add_custom_command(TARGET csmm POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E tar xf ${CMAKE_CURRENT_BINARY_DIR}/wit.zip
        COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_CURRENT_BINARY_DIR}/wit
        COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_CURRENT_BINARY_DIR}/wit-v3.05a-r8638-cygwin64 ${CMAKE_CURRENT_BINARY_DIR}/wit
    )
    file(DOWNLOAD https://szs.wiimm.de/download/szs-v2.40b-r8923-cygwin64.zip ${CMAKE_CURRENT_BINARY_DIR}/szs.zip)
    add_custom_command(TARGET csmm POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E tar xf ${CMAKE_CURRENT_BINARY_DIR}/szs.zip
        COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_CURRENT_BINARY_DIR}/szs
        COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_CURRENT_BINARY_DIR}/szs-v2.40b-r8923-cygwin64 ${CMAKE_CURRENT_BINARY_DIR}/szs
    )
    add_custom_command(TARGET csmm PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${Python3_RUNTIME_LIBRARY_DIRS}/python${Python3_VERSION_MAJOR}${Python3_VERSION_MINOR}.dll ${PYTHON_STDLIB_DESTFOLDER}
    )
else()
    add_executable(csmm ${MAIN_SOURCES} ${QM_FILES})
    add_executable(csmmpython ${PY_SOURCES} ${QM_FILES})

    file(DOWNLOAD https://wit.wiimm.de/download/wit-v3.05a-r8638-x86_64.tar.gz ${CMAKE_CURRENT_BINARY_DIR}/wit.tar.gz)
    add_custom_command(TARGET csmm POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E tar xzf ${CMAKE_CURRENT_BINARY_DIR}/wit.tar.gz
        COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_CURRENT_BINARY_DIR}/wit
        COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_CURRENT_BINARY_DIR}/wit-v3.05a-r8638-x86_64 ${CMAKE_CURRENT_BINARY_DIR}/wit
    )
    file(DOWNLOAD https://szs.wiimm.de/download/szs-v2.40b-r8923-x86_64.tar.gz ${CMAKE_CURRENT_BINARY_DIR}/szs.tar.gz)
    add_custom_command(TARGET csmm POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E tar xzf ${CMAKE_CURRENT_BINARY_DIR}/szs.tar.gz
        COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_CURRENT_BINARY_DIR}/szs
        COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_CURRENT_BINARY_DIR}/szs-v2.40b-r8923-x86_64 ${CMAKE_CURRENT_BINARY_DIR}/szs
    )
endif()

add_custom_target(translations ALL DEPENDS ${qm_files})
add_dependencies(csmm translations)

set_target_properties(csmm PROPERTIES CXX_VISIBILITY_PRESET hidden)

target_precompile_headers(csmm PRIVATE
    "$<$<COMPILE_LANGUAGE:CXX>:<QVector$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<QString$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<QFile$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<QDataStream$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<QDebug$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<pybind11/pybind11.h$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<vector$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<algorithm$<ANGLE-R>>"
)

target_compile_definitions(csmm PUBLIC CSMM_VERSION="${PROJECT_VERSION}")

if(NOT WIN32)
    find_package(yaml-cpp REQUIRED)

    if(APPLE)
        set(YAML_CPP_LIBRARIES "yaml-cpp::yaml-cpp")
    else()
        set(YAML_CPP_LIBRARIES "yaml-cpp")
    endif()

    target_include_directories(csmm PRIVATE ${YAML_CPP_INCLUDE_DIR})
    target_include_directories(csmmpython PRIVATE ${YAML_CPP_INCLUDE_DIR})
    target_link_libraries(csmm PRIVATE ${YAML_CPP_LIBRARIES})
    target_link_libraries(csmmpython PRIVATE ${YAML_CPP_LIBRARIES})
else()
    add_subdirectory(lib/yaml-cpp)
    target_include_directories(csmm PRIVATE lib/yaml-cpp/include)
    target_include_directories(csmmpython PRIVATE lib/yaml-cpp/include)
    target_link_libraries(csmm PRIVATE pybind11::windows_extras yaml-cpp)
    target_link_libraries(csmmpython PRIVATE pybind11::windows_extras yaml-cpp)
endif()

add_subdirectory(lib/pybind11)
add_subdirectory(lib/libbecquerel)
target_include_directories(csmm PRIVATE lib/libbecquerel)
target_include_directories(csmmpython PRIVATE lib/libbecquerel)

target_link_libraries(csmm PRIVATE
    becquerel
    pybind11::embed
    Qt6::Concurrent
    Qt6::Core
    Qt6::Gui
    Qt6::Network
    Qt6::Widgets
)

target_link_libraries(csmmpython PRIVATE
    becquerel
    pybind11::embed
    Qt6::Concurrent
    Qt6::Core
    Qt6::Gui
    Qt6::Network
    Qt6::Widgets
)

set_target_properties(csmm PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER com.fortunestreetmodding
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE FALSE
)
