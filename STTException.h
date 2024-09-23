#ifndef STTEXCEPTION_H_
#define STTEXCEPTION_H_

/**
 * @file STTException.h
 * @brief This declares and defines the `STTException` class
 */

#include <stdexcept>

namespace stt {
    class STTException;
}

/// this represents a STT runtimeerror
class stt::STTException: public std::runtime_error
{
public:
    STTException(const char *message): std::runtime_error(message) {}
};

#endif /* STTEXCEPTION_H_ */
