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

#include "utils_mac.h"
#include <Foundation/Foundation.h>
#include <AppKit/AppKit.h>

#include <string>

#include <QDebug>

namespace ria_tera
{
    
struct MacUtilsImpl {
    ;
};

MacUtils::MacUtils() : impl(new MacUtilsImpl) {
    //impl->wrapped = [[ABCWidget alloc] init];
}

MacUtils::~MacUtils()
{
    //if (impl) [impl->wrapped release];
    delete impl;
}

bool MacUtils::askPermissions(char const* path) {
    // https://stackoverflow.com/questions/12153504/accessing-the-desktop-in-a-sandboxed-app
    std::string url;
    url.append("file://");
    url.append(path);
    
    char const* url_path = url.c_str();

    NSString *nsUrlStr = [NSString stringWithUTF8String:url_path];
    NSURL *bookmarkedURL = [NSURL URLWithString:nsUrlStr];
    BOOL ok = [bookmarkedURL startAccessingSecurityScopedResource];
    NSLog(@"Accessed ok: %d %@", ok, [bookmarkedURL relativePath]);
    return ok;
}

}; // namespace

