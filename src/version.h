/*
 * TeRa
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef VERSION_H
#define VERSION_H

namespace ria_tera
{
// version number controlled from  cmake/modules/TeRaVersionInfo.cmake
static QString TERA_CLIENT_VERSION = QString("%1.%2.%3.%4").arg(MAJOR_VER).arg(MINOR_VER).arg(RELEASE_VER).arg(BUILD_VER);
static QString TERA_TOOL_VERSION = QString("%1.%2.%3.%4").arg(MAJOR_VER).arg(MINOR_VER).arg(RELEASE_VER).arg(BUILD_VER);
}

#endif // VERSION_H