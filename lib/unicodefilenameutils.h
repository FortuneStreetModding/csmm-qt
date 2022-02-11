#ifndef UNICODEFILENAMEUTILS_H
#define UNICODEFILENAMEUTILS_H

#include "filesystem.hpp"
#include <QString>
#if defined(__GNUC__) && defined(Q_OS_WIN)
#include <ext/stdio_filebuf.h>
#endif

namespace ufutils {

class unicode_ifstream : public ghc::filesystem::ifstream {
#if defined(__GNUC__) && defined(Q_OS_WIN)
    __gnu_cxx::stdio_filebuf<char> buf;
    std::streambuf *oldBuf;
public:
    unicode_ifstream(const QString &fileName) : ghc::filesystem::ifstream(), buf(_wfopen(fileName.toStdWString().c_str(), L"rb"), std::ios::in) {
        oldBuf = std::ios::rdbuf(&buf);
    }
    ~unicode_ifstream() {
        std::ios::rdbuf(oldBuf);
        buf.close();
    }
#else
public:
    unicode_ifstream(const QString &fileName) : ghc::filesystem::ifstream(fileName.toStdU16String(), std::ios::in) {}
#endif
};

};

#endif // UNICODEFILENAMEUTILS_H
