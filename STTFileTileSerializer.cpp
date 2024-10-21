/**
* @file STTFileTileSerializer.cpp
* @brief this defines the `STTFileTileSerializer` class
*/

#include <cstdio>
#include <cstring>
#include <mutex>

#include "../deps/concat.h"
#include "cpl_vsi.h"
#include "STTException.h"
#include "STTFileTileSerializer.h"

#include "STTFileOutputStream.h"
#include "STTZOutputStream.h"

using namespace std;
using namespace stt;

/// create a filename for a tile coordinate
std::string
stt::STTFileTileSerializer::getTileFilename(const TileCoordinate *coord, const string dirname, const char *extension)
{
    static mutex mutex;
    VSIStatBufL stat;
    string filename = 
}
