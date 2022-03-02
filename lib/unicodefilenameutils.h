#ifndef UNICODEFILENAMEUTILS_H
#define UNICODEFILENAMEUTILS_H

#include "filesystem.hpp"
#include <QString>
#if defined(Q_CC_GNU) && defined(Q_OS_WIN)
#include <ext/stdio_filebuf.h>
#endif

namespace ufutils {

class unicode_ifstream : public ghc::filesystem::ifstream {
#if defined(Q_CC_GNU) && defined(Q_OS_WIN)
    __gnu_cxx::stdio_filebuf<char> buf;
    std::streambuf *oldBuf;
public:
    unicode_ifstream(const QString &fileName) : ghc::filesystem::ifstream(), buf(_wfopen(fileName.toStdWString().c_str(), L"rb"), std::ios::in | std::ios::binary) {
        oldBuf = std::ios::rdbuf(&buf);
    }
    ~unicode_ifstream() {
        std::ios::rdbuf(oldBuf);
        buf.close();
    }
#else
public:
    unicode_ifstream(const QString &fileName) : ghc::filesystem::ifstream(fileName.toStdU16String(), std::ios::in | std::ios::binary) {}
#endif
};

class unicode_ofstream : public ghc::filesystem::ofstream {
#if defined(Q_CC_GNU) && defined(Q_OS_WIN)
    __gnu_cxx::stdio_filebuf<char> buf;
    std::streambuf *oldBuf;
public:
    unicode_ofstream(const QString &fileName) : ghc::filesystem::ofstream(), buf(_wfopen(fileName.toStdWString().c_str(), L"wb"), std::ios::out | std::ios::binary) {
        oldBuf = std::ios::rdbuf(&buf);
    }
    ~unicode_ofstream() {
        std::ios::rdbuf(oldBuf);
        buf.close();
    }
#else
public:
    unicode_ofstream(const QString &fileName) : ghc::filesystem::ofstream(fileName.toStdU16String(), std::ios::out | std::ios::binary) {}
#endif
};

class unicode_fstream : public ghc::filesystem::fstream {
#if defined(Q_CC_GNU) && defined(Q_OS_WIN)
    __gnu_cxx::stdio_filebuf<char> buf;
    std::streambuf *oldBuf;
public:
    unicode_fstream(const QString &fileName) : ghc::filesystem::fstream(), buf(_wfopen(fileName.toStdWString().c_str(), L"rb+"), std::ios::in | std::ios::out | std::ios::binary) {
        oldBuf = std::ios::rdbuf(&buf);
    }
    ~unicode_fstream() {
        std::ios::rdbuf(oldBuf);
        buf.close();
    }
#else
public:
    unicode_fstream(const QString &fileName) : ghc::filesystem::fstream(fileName.toStdU16String(), std::ios::in | std::ios::out | std::ios::binary) {}
#endif
};

};

#endif // UNICODEFILENAMEUTILS_H
