#include <iostream>
#include <filesystem>
#include <string>
#include <vector>

#include "boost/program_options.hpp"
#include "gdal_priv.h"

#include "GDALDatasetReader.h"

// using namespace std;
using namespace stt;
namespace po = boost::program_options;
namespace fs = std::filesystem;

int main(int argc, char *argv[])
{
    std::string version {"v0.1.0"};
    fs::path inputFile;
    fs::path outputDir;

    po::options_description optionalArgs;
    optionalArgs.add_options()
        ("help,h", "show help message")
        ("version,v", "show version")
        (
            "output-directory,o",
            po::value<fs::path>(&outputDir)->default_value(fs::current_path()),
            "output directory"
        )
    ;

    po::options_description hiddenArgs;
    hiddenArgs.add_options()
        ("input-file,i", po::value<fs::path>(&inputFile), "input file")
    ;

    po::options_description commandLineArgs;
    commandLineArgs.add(optionalArgs).add(hiddenArgs);

    po::positional_options_description positionArgs;
    positionArgs.add("input-file", -1);

    po::variables_map varMap;
    po::store(
        po::command_line_parser(argc, argv).
        options(commandLineArgs).
        positional(positionArgs).
        run(),
        varMap
    );
    po::notify(varMap);

    if (varMap.count("help")) {
        std::cout << "space-terrain-tiler " << version << "\n";
        std::cout << optionalArgs << "\n";
    }

    if (varMap.count("version")) {
        std::cout << "space-terrain-tiler " << version << "\n";
    }

    if (varMap.count("input-file")) {
        if (fs::is_regular_file(inputFile)) {
            std::cout << "input file: " << inputFile << "\n";
        } else {
            std::cerr << "input file " << inputFile << " not found\n";
            return EXIT_FAILURE;
        }
    }

    if (varMap.count("output-directory")) {
        if (fs::is_directory(outputDir)) {
            std::cout << "output directory: " << outputDir << "\n";
        } else {
            std::cout << "output directory " << outputDir;
            std::cout << " not found. using current path\n";
        }
    }

    GDALAllRegister();

    // define the grid we are going to use
    Grid grid1;
    int tileSize1 = { 65 };
    Grid grid2;
    int tileSize2 = { 256 };

    std::cout << "tileSize1: " << tileSize1 << "\n";
    std::cout << "tileSize2: " << tileSize2 << "\n";

    // OGRSpatialReference srs;
    // srs.importFromEPSG(4326);

    // grid = Grid(tileSize, CRSBounds(-180, -90, 180, 90), srs);

    // std::cout << grid.getSRS().exportToWkt() << "\n";

    grid1 = GlobalGeodetic(tileSize1);
    // grid2 = GlobalMercator(tileSize2);

    std::cout << grid1.tileSize() << "\n";
    std::cout << grid1.getSRS().exportToWkt() << "\n";
}
