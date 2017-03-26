#ifndef VERSION_H
#define VERSION_H

namespace ria_tera
{
// version number controlled from  cmake/modules/TeRaVersionInfo.cmake
static QString TERA_CLIENT_VERSION = QString("%1.%2.%3.%4").arg(MAJOR_VER).arg(MINOR_VER).arg(RELEASE_VER).arg(BUILD_VER);
static QString TERA_TOOL_VERSION = QString("%1.%2.%3.%4").arg(MAJOR_VER).arg(MINOR_VER).arg(RELEASE_VER).arg(BUILD_VER);
}

#endif // VERSION_H