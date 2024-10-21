/**
* @file STTZOutputStream.cpp
* @brief this defines the `STTZOutputStream` and `STTZFileOutputStream` classes
*/

#include "STTException.h"
#include "STTZOutputStream.h"

using namespace stt;

/**
* @details
* writes a sequence of memory pointed by ptr into the GZFile*.
*/
uint32_t
stt::STTZOutputStream::write(const void *ptr, uint32_t size)
{
    if (size == 1) {
        int c = *((const char *)ptr);
        return gzputc(fp, c) == -1 ? 0 : 1;
    } else {
        return gzwrite(fp, ptr, size) == 0 ? 0 : size;
    }
}

stt::STTZFileOutputStream::STTZFileOutputStream(const char *fileName): STTZOutputStream(NULL)
{
    gzFile file = gzopen(fileName, "wb");

    if (file == NULL) {
        throw STTException("Failed to open file");
    }

    fp = file;
}

stt::STTZFileOutputStream::~STTZFileOutputStream()
{
    close();
}

void
stt::STTZFileOutputStream::close()
{
    // try and close the file
    if (fp) {
        switch (gzclose(fp)) {
            case Z_OK:
                break;
            case Z_STREAM_ERROR:
            case Z_ERRNO:
            case Z_MEM_ERROR:
            case Z_BUF_ERROR:
            default:
                throw STTException("Failed to close file");
        }

        fp = NULL;
    }
}
