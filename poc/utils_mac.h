#pragma once

namespace ria_tera
{
    struct MacUtilsImpl;
    class MacUtils
    {
        MacUtilsImpl* impl;
    public:
        MacUtils();
        ~MacUtils();
        bool askPermissions(char const* path);
    };
}
