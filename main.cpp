#include <cmath>

#include "Utility.hpp"
#include "Logger.hpp"
#include "Decoder.hpp"

void printHelp()
{
    std::cout << "===========================================" << std::endl;
    std::cout << "   K-PEG - Simple JPEG Encoder & Decoder"    << std::endl;
    std::cout << "===========================================" << std::endl;
    std::cout << "Help\n" << std::endl;
    std::cout << "<filename.jpg>                  : Decompress a JPEG image to a PPM image" << std::endl;
    std::cout << "-h                              : Print this help message and exit" << std::endl;
}

void decodeJPEG(const std::string& filename)
{
    if ( !kpeg::utils::isValidFilename( filename ) )
    {
        LOG(kpeg::Logger::Level::ERROR) << "Invalid input file name passed." << std::endl;
        return;
    }
    
    kpeg::Decoder decoder;
    decoder.open( filename );
    if ( decoder.decodeImageFile() == kpeg::Decoder::ResultCode::DECODE_DONE )
    {
        decoder.dumpRawData();
    }
}

int handleInput(int argc, char** argv)
{
    if ( argc < 2 )
    {
        LOG(kpeg::Logger::Level::ERROR) << "No arguments provided." << std::endl;
        return EXIT_FAILURE;
    }
    
    if ( argc == 2 && (std::string)argv[1] == "-h" )
    {
        printHelp();
        return EXIT_SUCCESS;
    }
    else if ( argc == 2 )
    {
        decodeJPEG( argv[1] );
        return EXIT_SUCCESS;
    }
    
    LOG(kpeg::Logger::Level::ERROR) << "Incorrect usage, use -h to view help" << std::endl;
    return EXIT_FAILURE;
}