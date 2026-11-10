#pragma once
// Opening files by UTF-8 path and seeking past 2 GB, the same way on every system.
// Windows wants wide-character paths and its own 64-bit seek; elsewhere paths are UTF-8 already
// and off_t is 64 bits.
#include <cstdio>
#include <filesystem>
#include <string>

namespace pulp::fileio {

inline FILE* open(const std::filesystem::path& p, const char* mode) {
#ifdef _WIN32
    wchar_t wmode[8] = {};
    for (int i = 0; i < 7 && mode[i]; ++i) wmode[i] = (wchar_t)mode[i];
    return _wfopen(p.wstring().c_str(), wmode);
#else
    return std::fopen(p.c_str(), mode);
#endif
}

inline int seek(FILE* f, long long offset, int whence) {
#ifdef _WIN32
    return _fseeki64(f, offset, whence);
#else
    return fseeko(f, (off_t)offset, whence);
#endif
}

inline long long tell(FILE* f) {
#ifdef _WIN32
    return _ftelli64(f);
#else
    return (long long)ftello(f);
#endif
}

} // namespace pulp::fileio
