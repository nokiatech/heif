/* Copyright (c) 2015, Nokia Technologies Ltd.
 * All rights reserved.
 *
 * Licensed under the Nokia High-Efficiency Image File Format (HEIF) License (the "License").
 *
 * You may not use the High-Efficiency Image File Format except in compliance with the License.
 * The License accompanies the software and can be found in the file "LICENSE.TXT".
 *
 * You may also obtain the License at:
 * https://nokiatech.github.io/heif/license.txt
 */

#include "buildinfo.hpp"
#include "log.hpp"
#include "writerapp.hpp"

#include <cstdlib>
#include <iostream>

void printHelp(const std::string& executableName);

/**
 * @brief Handle arguments
 * @details Set up verbosity level, print help, etc.
 * @return True if file processing is not necessarily needed
 */
bool handleArguments(int argc, char *argv[]);
std::string findFilename(int argc, char *argv[]);

int main(int argc, char *argv[])
{
    try
    {
        const bool okToExit = handleArguments(argc, argv);
        const std::string inputFile = findFilename(argc, argv);

        if (inputFile.empty() && okToExit)
        {
            return EXIT_SUCCESS;
        }
        else if (inputFile.empty())
        {
            throw std::runtime_error("Input configuration file name is required.");
        }

        WriterApp writerApp;
        writerApp.processConfiguration(inputFile);
    }
    catch (std::exception& e)
    {
        logError() << "Error: " << e.what() << std::endl;
        printHelp(argv[0]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void printHelp(const std::string& executableName)
{
    std::cout << "Usage: " << executableName << " [options] configuration-file" << std::endl
        << "Supported options:" << std::endl
        << "--help           Print this help" << std::endl
        << "--verbose, -v    Print debug information" << std::endl
        << "--version        Print build version" << std::endl
        << "--no-warnings    Print errors only" << std::endl;
}

bool handleArguments(int argc, char *argv[])
{
    std::vector<std::string> parameters;
    for (int i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '-')
        {
            parameters.push_back(argv[i]);
        }
    }

    bool exitSuccess = false;
    for (auto iter : parameters)
    {
        if (iter == "-v" || iter == "--verbose")
        {
            Log::setLevel(Log::LogLevel::INFO);
        }
        else if (iter == "--help")
        {
            printHelp(argv[0]);
            exitSuccess = true;
        }
        else if (iter == "--version")
        {
            std::cout << "HEIF writer build version " << BuildInfo::Version << " date " << BuildInfo::Time << std::endl;
            exitSuccess = true;
        }
        else if (iter == "--no-warnings")
        {
            Log::setLevel(Log::LogLevel::ERROR);
        }
        else
        {
            throw std::runtime_error("Unknown parameter '" + iter + "'");
        }
    }

    return exitSuccess;
}

std::string findFilename(int argc, char *argv[])
{
    std::string filename;
    // Treat an argument without "-" prefix as the input configuration file name
    for (int i = 1; i < argc; ++i)
    {
        if (argv[i][0] != '-' && filename.empty())
        {
            filename = argv[i];
        }
        else if (argv[i][0] != '-' && not filename.empty())
        {
            throw std::runtime_error("Multiple input files?");
        }
    }

    return filename;
}
