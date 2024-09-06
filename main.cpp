#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include "boost/program_options.hpp"
#include "gdal_priv.h"

using namespace std;
namespace po = boost::program_options;
namespace fs = std::filesystem;

int main(int argc, char *argv[]){
    string version {"v0.1.0"};
    // vector<string> inputFiles;
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
        // ("input-file,i", po::value<vector<string>>(), "input file")
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

    // if (varMap.count("input-file")) {
    //     inputFiles = varMap["input-file"].as<vector<string>>();
    //     for (const string &i : inputFiles) {
    //         cout << i << "\n";
    //         // cout << "input files: " << varMap["input-file"].as<vector<std::string>>()[0] << "\n";
    //     }
    // }

    if (varMap.count("help")) {
        cout << "space-terrain-tiler " << version << "\n";
        cout << optionalArgs << "\n";
    }

    if (varMap.count("version")) {
        cout << "space-terrain-tiler " << version << "\n";
    }

    if (varMap.count("input-file")) {
        if (fs::is_regular_file(inputFile)) {
            cout << "input file: " << inputFile << "\n";
        } else {
            cerr << "input file " << inputFile << " not found\n";
            return EXIT_FAILURE;
        }
    }

    if (varMap.count("output-directory")) {
        if (fs::is_directory(outputDir)) {
            cout << "output directory: " << outputDir << "\n";
        } else {
            cout << "output directory " << outputDir;
            cout << " not found. using current path\n";
        }
    }

    GDALAllRegister();
}
