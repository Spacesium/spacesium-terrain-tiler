#ifndef STTCONFIG_H_
#define STTCONFIG_H_

/**
 * @file config.h
 * @brief build time configured options and settings
 *
 * this file is a template used by the CMake build system to create `config.h`
 */

/*
 * enable symbol export in visual studio 2013 as per issue #6. this is an
 * adaption of `CPL_DLL` in GDAL's `cpl_port.h`.
 */

#ifndef STT_DLL
#if defined(_MSC_VER)
#  if !defined(CPL_DISABLE_DLL)
#      define STT_DLL __declspec(dllexport)
#  else
#      define STT_DLL __declspec(dllimport)
#  endif // !defined(CPL_DISABLE_DLL)
#else
#  define STT_DLL
#endif // defined(_MSC_VER)
#endif // STT_DLL

#include <string>
#include <sstream>

namespace stt {

    /**
     * @brief versioning object
     * we use [semantic versioning](http://semver.org)
     */
    const struct semserver {
        /// the major version
        const unsigned short int major;
        /// the minor version
        const unsigned short int minor;
        /// the patch number
        const unsigned short int patch;

        /// the version string
        const char *cstr;
    } version = {
        0,
        1,
        0,
        "0.1.0"
    };

    /// the width and height of height data in a tile
    const unsigned short int TILE_SIZE = 65;

    /// the width and height of water mask data in a tile
    const unsigned short int MASK_SIZE = 256;
}

#endif /* STTCONFIG_H_ */
