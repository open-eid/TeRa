# === To update translation files ===
# (for further reading see https://itk.org/Wiki/CMake:How_To_Build_Qt4_Software)
# * Uncomment "SET (UPDATE_TRANSLATIONS 1)"
# * Build configuration with cmake
# * Build with cmake
# * NB! Comment "SET (UPDATE_TRANSLATIONS 1)" out again when not needed anymore and build new configuration with cmake
# (* Open&translate translations/*.ts with "Qt Linquist")
# (* at the end "File -> Release All" to generate binary versions that are compiled into executables)
# (* Build with cmake)
#SET (UPDATE_TRANSLATIONS 1)

cmake_minimum_required (VERSION 3.0)
if(POLICY CMP0043)
    cmake_policy(SET CMP0043 NEW)
endif()
if(POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif()
if(POLICY CMP0071)
    cmake_policy(SET CMP0071 NEW)
endif()

project(TeRa VERSION 1.1.0)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTOUIC ON) 
SET(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 14)
SET(CMAKE_OSX_DEPLOYMENT_TARGET "10.11.0")

include(VersionInfo)
if(WIN32)
    set_app_name(PROGNAME qdigidoc_tera)
    set(TERA_CMD_NAME qdigidoc_tera)
    set(TERA_GUI_NAME qdigidoc_tera_gui)
    set(TERA_SCARD_LIBRARIES Version SetupApi winscard NCrypt Crypt32)

    ADD_DEFINITIONS(-DZIP_STATIC)
    find_library(ZIP_LIBRARIES NAMES zipstatic HINTS $ENV{TERA_LIBZIP_DIR}/lib)
    SET (ZIP_INCLUDE_DIR "$ENV{TERA_LIBZIP_DIR}/include" "$ENV{TERA_LIBZIP_DIR}/lib/libzip/include")
elseif(APPLE)
    set_app_name(PROGNAME qdigidoc-tera)
    set(TERA_GUI_NAME ${PROGNAME})
    set(TERA_SCARD_LIBRARIES "-framework Cocoa -framework PCSC -fobjc-arc")

    ADD_DEFINITIONS(-DZIP_STATIC)
    find_library(ZIP_LIBRARIES NAMES zipstatic HINTS $ENV{TERA_LIBZIP_DIR}/lib)
    SET (ZIP_INCLUDE_DIR "$ENV{TERA_LIBZIP_DIR}/include" "$ENV{TERA_LIBZIP_DIR}/lib/libzip/include")
else()
    set_app_name(PROGNAME qdigidoc-tera)
    set(TERA_CMD_NAME qdigidoc-tera)
    set(TERA_GUI_NAME qdigidoc-tera-gui)

    find_package(PCSCLite REQUIRED)
    find_package(Threads)
    include_directories(${PCSCLITE_INCLUDE_DIR})
    set(TERA_SCARD_LIBRARIES ${PCSCLITE_LIBRARY})
    IF (DEFINED ENV{TERA_USE_UNIX_STORAGE_INFO})
        add_definitions(-DTERA_USE_UNIX_STORAGE_INFO)
    ENDIF()
    # find_package(LIBZIP REQUIRED)
    find_path(ZIP_INCLUDE_DIR__ZIP_H NAMES zip.h)
    find_path(ZIP_INCLUDE_DIR__ZIPCONF_H NAMES zipconf.h HINTS ${ZIP_INCLUDE_DIR__ZIP_H}/../lib/libzip/include/ )
    SET (ZIP_INCLUDE_DIR ${ZIP_INCLUDE_DIR__ZIP_H} ${ZIP_INCLUDE_DIR__ZIPCONF_H})
    find_library(ZIP_LIBRARIES NAMES zipstatic zip)
endif()

find_package(Qt5 COMPONENTS Core Widgets Network LinguistTools REQUIRED)
find_package(PKCS11)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR} ${ZIP_INCLUDE_DIR})

message(STATUS "Qt5Core version:     " ${Qt5Core_VERSION})
message(STATUS "ZLIB_INCLUDE_DIRS:   " ${ZLIB_INCLUDE_DIRS})
message(STATUS "ZLIB_LIBRARIES:      " ${ZLIB_LIBRARIES})
message(STATUS "ZIP_LIBRARIES:       " ${ZIP_LIBRARIES})
message(STATUS "ZIP_INCLUDE_DIR:     " ${ZIP_INCLUDE_DIR})
message(STATUS "OPENSSL_INCLUDE_DIR: " ${OPENSSL_INCLUDE_DIR})
message(STATUS "OPENSSL_LIBRARIES:   " ${OPENSSL_LIBRARIES})
message(STATUS "TERA VERSION:        " ${VERSION})
message(STATUS "RESOURCE_FILES:      " ${RESOURCE_FILES})

SET (TERA_COMMON_LIB_SRC
     common/QPCSC.h common/QPCSC.cpp common/QPCSC_p.h
     common/QSmartCard.h common/QSmartCard.cpp common/QSmartCard_p.h
     common/SslCertificate.h common/SslCertificate.cpp
     common/qasn1element_p.h common/qasn1element.cpp
     common/IKValidator.h common/IKValidator.cpp
     common/TokenData.h common/TokenData.cpp
     common/PinDialogInterface.h
     )
IF (WIN32)
    list(APPEND TERA_COMMON_LIB_SRC common/QCNG.h common/QCNG.cpp common/QWin.h common/QWin.cpp)
else ()
    list(APPEND TERA_COMMON_LIB_SRC common/QPKCS11.h common/QPKCS11.cpp common/QPKCS11_p.h)
endif()

SET (TERA_COMMON_LIB_GUI_SRC
     common/PinDialog.h common/PinDialog.cpp
    )

SET (TERA_COMMON_SRC
      src/version.h
      src/common/Bdoc10Handler.h src/common/Bdoc10Handler.cpp
      src/common/HttpsIDCardAuthentication.h src/common/HttpsIDCardAuthentication.cpp
     )

SET (TERA_LIBDIGIDOC_SRC
        src/libdigidoc/Configuration.h src/libdigidoc/Configuration.cpp
        src/libdigidoc/Settings.h)

SET (TERA_GUI_UI
        poc/MainWindow.ui
        poc/SettingsDialog.ui
        poc/FileListDialog.ui
        poc/IDCardSelectionDialog.ui
        common/AboutDialog.ui)
SET (TERA_GUI_SRC
        poc/xxx.cpp
        poc/main_window.h poc/main_window.cpp common/ComboBox.h common/ComboBox.cpp
        poc/gui_timestamper_processor.h poc/gui_timestamper_processor.cpp
        poc/settings_window.h poc/settings_window.cpp
        poc/files_window.h poc/files_window.cpp
        poc/id_card_select_dialog.h poc/id_card_select_dialog.cpp
        common/AboutDialog.h common/AboutDialog.cpp

        poc/utils.h poc/utils.cpp
        poc/openssl_utils.h poc/openssl_utils.cpp
        poc/disk_crawler.h poc/disk_crawler.cpp
        poc/logging.h poc/logging.cpp
        poc/timestamper.h poc/timestamper.cpp
        poc/config.h poc/config.cpp
     )

IF ( APPLE )
    SET (TERA_GUI_MAC_SRC
            poc/utils_mac.h
            poc/utils_mac.mm)
ENDIF()

SET (TRANS translations/et.ts translations/en.ts translations/ru.ts)
SET (TRANS_QT ${TRANS} translations/qtbase_et.ts translations/qtbase_ru.ts)

if (UPDATE_TRANSLATIONS)
    qt5_create_translation( QM ${TERA_GUI_UI} ${TERA_GUI_SRC} ${TRANS})
ENDIF()
configure_file( translations/tr.qrc tr.qrc COPYONLY )
qt5_add_translation( QM ${TRANS_QT} )
qt5_add_resources( CONFIG_SOURCES res/config.qrc )
qt5_add_resources( SOURCES images/images.qrc ${CMAKE_BINARY_DIR}/tr.qrc )
qt5_wrap_ui( UI_HEADERS ${TERA_GUI_UI} )

# TeRaTool
if(NOT APPLE)
    add_executable(${TERA_CMD_NAME}
        ${TERA_COMMON_SRC}
        poc/teracmdtool.rc
        poc/terapoc.cpp
        poc/utils.h poc/utils.cpp
        poc/openssl_utils.h poc/openssl_utils.cpp
        poc/disk_crawler.h poc/disk_crawler.cpp
        poc/logging.h poc/logging.cpp
        poc/timestamper.h poc/timestamper.cpp
        poc/config.h poc/config.cpp
        src/cmdtool/cmdline_timestamper_processor.h src/cmdtool/cmdline_timestamper_processor.cpp
        ${TERA_COMMON_LIB_SRC}
        ${TERA_LIBDIGIDOC_SRC}
        ${CONFIG_SOURCES}
        )
    target_link_libraries(${TERA_CMD_NAME}
        ${ZIP_LIBRARIES} ${OPENSSL_LIBRARIES} ${ZLIB_LIBRARIES} ${TERA_SCARD_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}
        Qt5::Network Qt5::Widgets)
endif()

# TeRa GUI
add_executable(${TERA_GUI_NAME} WIN32 MACOSX_BUNDLE
        ${TERA_COMMON_SRC}
        poc/teragui.rc
        ${UI_HEADERS}
        ${TERA_GUI_UI}
        ${TERA_GUI_SRC}
        ${TERA_GUI_MAC_SRC}
        ${SOURCES}
        ${TERA_LIBDIGIDOC_SRC}
        ${CONFIG_SOURCES}
        ${QM}
        ${TRANS_QT}
        ${TERA_COMMON_LIB_SRC}
        ${TERA_COMMON_LIB_GUI_SRC}
        ${RESOURCE_FILES}
        )
target_link_libraries(${TERA_GUI_NAME}
    ${ZIP_LIBRARIES} ${OPENSSL_LIBRARIES} ${ZLIB_LIBRARIES} ${TERA_SCARD_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}
    Qt5::Network Qt5::Widgets)

if( WIN32 )
    IF (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(PLATFORM "x64")
        set(OPENSSL_BIN_DIR "C:/Openssl-Win64")
    ELSE()
        set(PLATFORM "x86")
        set(OPENSSL_BIN_DIR "C:/Openssl-Win32")
    ENDIF()
    list(APPEND LIGHT_CMD "$ENV{WIX}bin\\light.exe" -sice:ICE80 tera.wixobj -ext WixUIExtension
        -dWixUILicenseRtf=${CMAKE_SOURCE_DIR}/cmake/modules/LICENSE.LGPL.rtf
        -dWixUIDialogBmp=${CMAKE_SOURCE_DIR}/dlgbmp.bmp -dWixUIBannerBmp=${CMAKE_SOURCE_DIR}/banner.bmp)
    add_custom_target( msi
        COMMAND "$ENV{WIX}bin\\candle.exe" ${CMAKE_SOURCE_DIR}/tera.wxs -dMSI_VERSION=${VERSION} -dPlatform=${PLATFORM}
            -dqt_path=${_qt5Core_install_prefix} -dopenssl_path=${OPENSSL_BIN_DIR} -dclient_path=${CMAKE_CURRENT_BINARY_DIR}
            -dqtconf=${CMAKE_SOURCE_DIR}/qt.conf
        COMMAND ${LIGHT_CMD} -loc ${CMAKE_SOURCE_DIR}/tera.en-US.wxl -cultures:en-US -out TeRa_${PLATFORM}.${VERSION}.en-US.msi
        COMMAND ${LIGHT_CMD} -loc ${CMAKE_SOURCE_DIR}/tera.et-EE.wxl -cultures:et-EE -out TeRa_${PLATFORM}.${VERSION}.et-EE.msi
    )
    if(SIGNCERT)
        list(APPEND SIGNCMD signtool.exe sign /a /v /s MY /n "${SIGNCERT}" /fd SHA256
            /du http://installer.id.ee /tr http://sha256timestamp.ws.symantec.com/sha256/timestamp /td SHA256)
        add_custom_command(TARGET ${TERA_CMD_NAME} POST_BUILD
            COMMAND ${SIGNCMD} $<TARGET_FILE:${TERA_CMD_NAME}>
        )
        add_custom_command(TARGET ${TERA_GUI_NAME} POST_BUILD
            COMMAND ${SIGNCMD} $<TARGET_FILE:${TERA_GUI_NAME}>
        )
        add_custom_command(TARGET msi POST_BUILD
            COMMAND ${SIGNCMD} "TeRa_${PLATFORM}.${VERSION}.en-US.msi" "TeRa_${PLATFORM}.${VERSION}.et-EE.msi"
        )
    endif()
elseif( APPLE )
    add_custom_command( TARGET ${PROGNAME} POST_BUILD
        COMMAND cp -a ${PKCS11_MODULE} $<TARGET_FILE_DIR:${TERA_GUI_NAME}>
    )
    get_filename_component( PKCS11_NAME ${PKCS11_MODULE} NAME )
    add_custom_target( macdeployqt DEPENDS ${TERA_GUI_NAME}
        COMMAND ${_qt5Core_install_prefix}/bin/macdeployqt ${CMAKE_CURRENT_BINARY_DIR}/${TERA_GUI_NAME}.app
            -executable=${CMAKE_CURRENT_BINARY_DIR}/${PROGNAME}.app/Contents/MacOS/${PKCS11_NAME}
    )
    add_custom_target( codesign DEPENDS ${TERA_GUI_NAME}
        COMMAND codesign -f -s \"$$SIGNCERT\"
            $<TARGET_FILE_DIR:${PROGNAME}>/*.*
            $<TARGET_FILE_DIR:${TERA_GUI_NAME}>/../Frameworks/*
            $<TARGET_FILE_DIR:${TERA_GUI_NAME}>/../PlugIns/*/*
        COMMAND codesign -f -v -s \"$$SIGNCERT\" $<TARGET_FILE_DIR:${TERA_GUI_NAME}>/../..
            --entitlements ${CMAKE_SOURCE_DIR}/${TERA_GUI_NAME}.entitlements
    )
    add_custom_target( zip DEPENDS ${TERA_GUI_NAME}
        COMMAND zip --symlinks -r -D ${CMAKE_BINARY_DIR}/${TERA_GUI_NAME}_${VERSION}$ENV{VER_SUFFIX}.zip ${TERA_GUI_NAME}.app
    )
    add_custom_target( productbuild DEPENDS ${TERA_GUI_NAME}
        COMMAND productbuild --component ${CMAKE_CURRENT_BINARY_DIR}/${TERA_GUI_NAME}.app /Applications
            --sign \"$$INSTCERT\" ${TERA_GUI_NAME}-store_${VERSION}$ENV{VER_SUFFIX}.pkg
    )
else()
    include( GNUInstallDirs )
    configure_file( qdigidoc-tera.1.cmake qdigidoc-tera.1 )
    configure_file( qdigidoc-tera-gui.1.cmake qdigidoc-tera-gui.1 )
    install( TARGETS ${TERA_CMD_NAME} ${TERA_GUI_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR} )
    install( FILES ${CMAKE_CURRENT_BINARY_DIR}/qdigidoc-tera.1 ${CMAKE_CURRENT_BINARY_DIR}/qdigidoc-tera-gui.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 )
    install( FILES qdigidoc-tera.appdata.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/appdata RENAME ee.tera.qdigidoc-tera.desktop )
    install( FILES qdigidoc-tera.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications RENAME ee.tera.qdigidoc-tera.desktop )
    foreach( RES 16x16 32x32 48x48 128x128 )
        install(
            FILES images/tera_icon_${RES}.png
            DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/${RES}/apps/
            RENAME qdigidoc-tera.png
        )
    endforeach(RES)
endif()
