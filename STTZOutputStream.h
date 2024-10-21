#ifndef STTZOUTPUTSTREAM_H_
#define STTZOUTPUTSTREAM_H_

/**
 * @file STTZOutputStream.h
 * @brief this declares and defines `STTZOutputStream` class
 */

#include "zlib.h"
#include "STTOutputStream.h"

namespace stt {
    class STTZFileOutputStream;
    class STTZOutputStream;
}

/// implements STTOutputStream for `GZFile` object
class STT_DLL stt::STTZOutputStream: public stt::STTOutputStream
{
public:
    STTZOutputStream(gzFile gzptr): fp(gzptr) {}

    /// writes a sequence of memory pointed by ptr into the stream
    virtual uint32_t write(const void *ptr, uint32_t size);

protected:
    /// the underlying GZFILE*
    gzFile fp;
}

/// implements STTOutputStream for gzipped files
class STT_DLL stt::STTZFileOutputStream: public stt::STTZOutputStream
{
    STTZFileOutputStream(const char *filename);
    ~STTZFileOutputStream();

    void close();
}

#endif /* STTZOUTPUTSTREAM_H_ */
