#ifndef STTOUTPUTSTREAM_H_
#define STTOUTPUTSTREAM_H_

/**
 * @file STTOutputStream.h
 * @brief this declares and defines the `STTOutputStream` class
 */

#include "config.h"
#include "types.h"

namespace stt {
    class STTOutputStream;
}

/// this represents a generic STT output stream to write raw data
class STT_DLL stt::STTOutputStream
{
public:
    /// writes a sequence of memory pointed by ptr into the stream
    virtual uint32_t write(const void *ptr, uint32_t size) = 0;
};

#endif /* STTOUTPUTSTREAM_H_ */
