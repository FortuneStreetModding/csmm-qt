# FindZIP.cmake - Pack and unpack ZIP files as custom targets
# ------------------------------------------------------------------------------
# Provides:
#
# ZIP_PACK_TARGET TARGET WORKING_DIRECTORY ARCHIVE FILES...
#	TARGET - The name of the custom target
#	WORKING_DIRECTORY - The working directory for the zip command
#	ARCHIVE - The full archive path
#	FILES... - The files and directories to include in the archive
#	           (relative to WORKING_DIRECTORY)
#
# ZIP_UNPACK_TARGET TARGET WORKING_DIRECTORY ARCHIVE FILES...
#	TARGET - The name of the custom target
#	WORKING_DIRECTORY - The working directory for the unzip command
#	ARCHIVE - The full archive path
#	FILES... - The files and directories to extract from the archive
#	           (default everything)
#	
# ------------------------------------------------------------------------------
# Copyright (c) Kevin Harrison 2016
# Released under the MIT License (https://opensource.org/licenses/MIT)

if (WIN32)
	set(ZIP_PROGRAMS "7z" "jar")
else (WIN32)
	set(ZIP_PROGRAMS "zip" "jar" "7z")
endif (WIN32)

set(ZIP_PATH_7z "$ENV{ProgramFiles}/7-Zip")
set(ZIP_PACK_7z "7z")
set(ZIP_UNPACK_7z "7z")
set(ZIP_PACK_FLAGS_7z "a" "-tzip")
set(ZIP_UNPACK_FLAGS_7z "x" "-tzip")

set(ZIP_PATH_jar "$ENV{JDKroot}/bin")
set(ZIP_PACK_jar "jar")
set(ZIP_UNPACK_jar "jar")
set(ZIP_PACK_FLAGS_jar "cf")
set(ZIP_UNPACK_FLAGS_jar "xf")

set(ZIP_PATH_zip "/usr/bin")
set(ZIP_PACK_zip "zip")
set(ZIP_UNPACK_zip "unzip")
set(ZIP_PACK_FLAGS_zip "-r" "-9" "-q")
set(ZIP_UNPACK_FLAGS_zip "")

foreach (zip ${ZIP_PROGRAMS})
	if (NOT ZIP_FOUND)
		find_program(ZIP_FOUND_${ZIP_PACK_${zip}} "${ZIP_PACK_${zip}}" PATHS "${ZIP_PATH_${zip}}" ENV "PATH")
		find_program(ZIP_FOUND_${ZIP_UNPACK_${zip}} "${ZIP_UNPACK_${zip}}" PATHS "${ZIP_PATH_${zip}}" ENV "PATH")
		if (ZIP_FOUND_${ZIP_PACK_${zip}} AND ZIP_FOUND_${ZIP_UNPACK_${zip}})
			set(ZIP_FOUND "${zip}")
		endif (ZIP_FOUND_${ZIP_PACK_${zip}} AND ZIP_FOUND_${ZIP_UNPACK_${zip}})
	endif (NOT ZIP_FOUND)
endforeach (zip ${ZIP_PROGRAMS})

if (NOT ZIP_FOUND)
	message(FATAL_ERROR "Failed to find zip program (7z, jar, or zip/unzip)")
endif (NOT ZIP_FOUND)

function (ZIP_PACK_TARGET TARGET WORKING_DIRECTORY ARCHIVE)
	set(DEPS "${WORKING_DIRECTORY}/${arg}")
	foreach (arg ${ARGN})
		file(GLOB_RECURSE glob "${WORKING_DIRECTORY}/${arg}/*")
		foreach (dep ${glob})
			list(APPEND DEPS "${dep}")
		endforeach (dep ${glob})
	endforeach (arg ${ARGN})
	file(RELATIVE_PATH relative "${CMAKE_BINARY_DIR}" "${ARCHIVE}")
	add_custom_command(
		OUTPUT "${ARCHIVE}"
		COMMAND "${ZIP_FOUND_${ZIP_PACK_${ZIP_FOUND}}}" ${ZIP_PACK_FLAGS_${ZIP_FOUND}} "${ARCHIVE}" ${ARGN}
		DEPENDS ${DEPS}
		WORKING_DIRECTORY "${WORKING_DIRECTORY}"
		COMMENT "Generating ${relative}")
	add_custom_target(
		"${TARGET}"
		ALL DEPENDS "${ARCHIVE}")
endfunction (ZIP_PACK_TARGET TARGET WORKING_DIRECTORY ARCHIVE)

function (ZIP_UNPACK_TARGET TARGET WORKING_DIRECTORY ARCHIVE)
	file(RELATIVE_PATH relative_bin "${CMAKE_BINARY_DIR}" "${ARCHIVE}")
	file(RELATIVE_PATH relative_src "${CMAKE_SOURCE_DIR}" "${ARCHIVE}")
	string(LENGTH "${relative_bin}" rel_bin_len)
	string(LENGTH "${relative_src}" rel_src_len)
	if ("${rel_bin_len}" GREATER "${rel_src_len}")
		set(relative "${relative_src}")
	else ("${rel_bin_len}" GREATER "${rel_src_len}")
		set(relative "${relative_bin}")
	endif ("${rel_bin_len}" GREATER "${rel_src_len}")
	add_custom_target(
		"${TARGET}"
		ALL "${ZIP_FOUND_${ZIP_UNPACK_${ZIP_FOUND}}}" ${ZIP_UNPACK_FLAGS_${ZIP_FOUND}} "${ARCHIVE}" ${ARGN}
		DEPENDS "${ARCHIVE}"
		WORKING_DIRECTORY "${WORKING_DIRECTORY}"
		COMMENT "Unpacking ${relative}")
endfunction (ZIP_UNPACK_TARGET TARGET WORKING_DIRECTORY ARCHIVE)