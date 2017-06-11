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

