#ifndef STTFILEOUTPUTSTREAM_H_
#define STTFILEOUTPUTSTREAM_H_

/**
 * @file STTFileOutputStream.h
 * @brief this declares and defines the `STTFileOutputStream` and `STTStdOutputStream` classes
 */

#include <cstdio>
#include <ostream>
#include "STTOutputStream.h"

namespace stt {
    class STTFileOutputStream;
    class STTStdOutputStream;
}

/// implements STTOutputStream for `FILE*` objects
class STT_DLL stt::STTFileOutputStream: public stt::STTOutputStream
{
public:
    STTFileOutputStream(FILE *fptr): fp(fptr) {}

    /// writes a sequence of memory pointed by ptr into the stream
    virtual uint32_t write(const void *ptr, uint32_t size);

protected:
    /// the underlying FILE*
    FILE *fp;
};

/// implements STTOutputStream for `std::ostream` objects
class STT_DLL stt::STTStdOutputStream: public stt::STTOutputStream
{
public:
    STTStdOutputStream(std::ostream &stream): mstream(stream) {}

    /// writes a sequence of memory pointed by ptr into the stream
    virtual uint32_t write(const void *ptr, uint32_t size);

protected:
    /// the underlying std::ostream
    std::ostream &mstream;
};

#endif /* STTFILEOUTPUTSTREAM_H_ */
