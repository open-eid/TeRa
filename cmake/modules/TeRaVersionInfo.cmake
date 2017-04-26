if(PROJECT_VERSION_MAJOR)
	set( MAJOR_VER ${PROJECT_VERSION_MAJOR} )
else()
	set( MAJOR_VER 1 )
endif()
if(PROJECT_VERSION_MINOR)
	set( MINOR_VER ${PROJECT_VERSION_MINOR} )
else()
	set( MINOR_VER 0 )
endif()
if(PROJECT_VERSION_PATCH)
	set( RELEASE_VER ${PROJECT_VERSION_PATCH} )
else()
	set( RELEASE_VER 0 )
endif()
if( DEFINED ENV{BUILD_NUMBER} )
	set( BUILD_VER $ENV{BUILD_NUMBER} )
elseif(PROJECT_VERSION_TWEAK)
	set( BUILD_VER ${PROJECT_VERSION_TWEAK} )
else()
	set( BUILD_VER 0 )
endif()
if(NOT BUILD_DATE)
	string(TIMESTAMP BUILD_DATE "%d.%m.%Y")
endif()

set( VERSION ${MAJOR_VER}.${MINOR_VER}.${RELEASE_VER}.${BUILD_VER} )
add_definitions(
	-DMAJOR_VER=${MAJOR_VER}
	-DMINOR_VER=${MINOR_VER}
	-DRELEASE_VER=${RELEASE_VER}
	-DBUILD_VER=${BUILD_VER}
	-DVER_SUFFIX=\"$ENV{VER_SUFFIX}\"
	-DBUILD_DATE=\"${BUILD_DATE}\"
	-DDOMAINURL=\"ria.ee\"
	-DORG=\"RIA\"
)

set( MACOSX_BUNDLE_COPYRIGHT "(C) 2017-2017 Estonian Information System Authority" )
set( MACOSX_BUNDLE_SHORT_VERSION_STRING ${MAJOR_VER}.${MINOR_VER}.${RELEASE_VER} )
set( MACOSX_BUNDLE_BUNDLE_VERSION ${BUILD_VER} )
set( MACOSX_BUNDLE_ICON_FILE Icon.icns )
set( MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${MAJOR_VER}.${MINOR_VER}.${RELEASE_VER} )
set( MACOSX_FRAMEWORK_BUNDLE_VERSION ${BUILD_VER} )
if( APPLE AND NOT IOS AND NOT CMAKE_OSX_DEPLOYMENT_TARGET )
	execute_process(COMMAND xcodebuild -version -sdk macosx SDKVersion
		OUTPUT_VARIABLE CMAKE_OSX_DEPLOYMENT_TARGET OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()




macro( SET_APP_NAME OUTPUT NAME )
	set( ${OUTPUT} "${NAME}" )
	add_definitions( -DAPP=\"${NAME}\" )
	set( MACOSX_BUNDLE_BUNDLE_NAME ${NAME} )
	set( MACOSX_BUNDLE_GUI_IDENTIFIER "ee.ria.${NAME}" )
	if( APPLE )
		file( GLOB_RECURSE TERA_MAC_RESOURCE_FILES
			${CMAKE_CURRENT_SOURCE_DIR}/mac/Resources/*.icns
			${CMAKE_CURRENT_SOURCE_DIR}/mac/Resources/*.strings )
		foreach( _file ${TERA_MAC_RESOURCE_FILES} )
			get_filename_component( _file_dir ${_file} PATH )
			file( RELATIVE_PATH _file_dir ${CMAKE_CURRENT_SOURCE_DIR}/mac ${_file_dir} )
			set_source_files_properties( ${_file} PROPERTIES MACOSX_PACKAGE_LOCATION ${_file_dir} )
		endforeach( _file )
	endif( APPLE )
endmacro()

macro( add_manifest TARGET )
	if( WIN32 )
		add_custom_command(TARGET ${TARGET} POST_BUILD
			COMMAND mt -nologo -manifest "${CMAKE_MODULE_PATH}/win81.exe.manifest" -outputresource:"$<TARGET_FILE:${TARGET}>")
	endif()
endmacro()

macro( SET_ENV NAME DEF )
	if( DEFINED ENV{${NAME}} )
		set( ${NAME} $ENV{${NAME}} ${ARGN} )
	else()
		set( ${NAME} ${DEF} ${ARGN} )
	endif()
endmacro()
