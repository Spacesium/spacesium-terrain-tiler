/**
* @file STTFileOutputStream.cpp
* @brief this defines the `STTFileOutputStream` class
*/

#include "STTFileOutputStream.h"

using namespace stt;

/**
* @details
* writes a sequence of memory pointed by ptr into the FILE*.
*/

uint32_t
stt::STTFileOutputStream::write(const void *ptr, uint32_t size) {
    return(uint32_t)fwrite(ptr, size, 1, fp);
}

/**
* @details
* writes a sequence of memory pointed by ptr into the ostream.
*/
uint32_t
stt::STTStdOutputStream::write(const void *ptr, uint32_t size) {
    mstream.write((const char *)ptr, size);
    return size;
}
