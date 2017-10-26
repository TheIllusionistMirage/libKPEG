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
    std::cout << "<filename.jpg>                  : Specify an input JPEG file to display" << std::endl;
    std::cout << "<filename.png> <filename.jpg>   : Convert input PNG file to JPEG" << std::endl;
    std::cout << "-h                              : Print this help message and exit" << std::endl;
}

void decodeJPEG(const std::string& filename)
{
    if ( !kpeg::isValidFilename( filename ) )
    {
        LOG(kpeg::Logger::Level::ERROR) << "Invalid input file name passed." << std::endl;
        return;
    }
    
}

void encodeImage(const std::string& filenameIn, const std::string& filenameOut)
{
    
}

int handleInput(int argc, char** argv)
{
    if ( argc < 2 )
    {
        LOG(kpeg::Logger::Level::ERROR) << "No arguments provided." << std::endl;
        return EXIT_FAILURE;
    }
    
    if ( argc == 2 )
    {
        decodeJPEG( argv[1] );
        return EXIT_SUCCESS;
    }
    else if ( argc == 3 )
    {
        encodeImage( argv[1], argv[2] );
    }
    else if ( argc == 2 && (std::string)argv[1] == "-h" )
    {
        printHelp();
        return EXIT_SUCCESS;
    }
    
    return EXIT_SUCCESS;
}

// tests

void huffmanTreeTest();
std::array<std::array<float, 8>, 8> DCTTest( std::array<std::array<int, 8>, 8>& coeffs );
std::array<std::array<float, 8>, 8> IDCTTest( std::array<std::array<float, 8>, 8>& coeffs );
void transformTest();
void colorTest();

int main( int argc, char** argv )
{
    try
    {
        // Output log file
        std::ofstream logFile( "kpeg.log", std::ios::out );
        
        // Set log tee streams to both the log file and STDOUT
        kpeg::TeeStream logTee( logFile, std::cout );
        
        // If the log file was created and can be written to,
        // log directly into the tee stream. Else log only to STDOUT        
        if ( logFile.is_open() && logFile.good() )
            kpeg::Logger::get().setLogStream( logTee );
        else
            kpeg::Logger::get().setLogStream( std::cout );
        
        kpeg::Logger::get().setLevel( kpeg::Logger::Level::DEBUG );
        
        LOG(kpeg::Logger::Level::INFO) << "KPEG - Simple JPEG Encoder & Decoder" << std::endl;

        ////////////////////////////
         
        kpeg::JPEGDecoder decoder;
        //decoder.open(argv[1]);
        //decoder.open("sample2.jpg");
        decoder.open("block.jpg");
        //decoder.printDetectedSegmentNames();
        if ( decoder.decodeImageFile() == kpeg::JPEGDecoder::ResultCode::DECODE_DONE )
        {
            decoder.displayImage();
        }
        //decoder.close();
         
        ////////////////////////////
        
         //huffmanTreeTest();
        
        ///////////////
        
        //transformTest();
        
        ///////////////
        
        //colorTest();
        
        ///////////////
        
//         std::cout << 17 << ": " << kpeg::valueToBitString( 17 ) << std::endl;
//         std::cout << 128 << ": " << kpeg::valueToBitString( 128 ) << std::endl;
//         std::cout << 65 << ": " << kpeg::valueToBitString( 65 ) << std::endl;
//         std::cout << 30 << ": " << kpeg::valueToBitString( 30 ) << std::endl;
//         std::cout << -34 << ": " << kpeg::valueToBitString( -34 ) << std::endl;
//         std::cout << -512 << ": " << kpeg::valueToBitString( -512 ) << std::endl;
        
//         std::cout << "000" << ": " << kpeg::bitStringtoValue( "000" ) << std::endl;
//         std::cout << "011101" << ": " << kpeg::bitStringtoValue( "011101" ) << std::endl;
//         std::cout << "11110" << ": " << kpeg::bitStringtoValue( "11110" ) << std::endl;
//         std::cout << "00101" << ": " << kpeg::bitStringtoValue( "00101" ) << std::endl;
//         std::cout << "10001" << ": " << kpeg::bitStringtoValue( "10001" ) << std::endl;
        
        //return handleInput(argc, argv);
    }
    catch( std::exception& e )
    {
        std::cout << "Exceptions Occurred:-" << std::endl;
        std::cout << "What: " << e.what() << std::endl;
    }
    
    return EXIT_SUCCESS;
}

void huffmanTreeTest()
{
    kpeg::HuffmanTable htable;
    htable[0].first = 0;
    htable[0].second = {};
    
    htable[1].first = 2;
    htable[1].second = { 0x01, 0x02 };
    
    htable[2].first = 1;
    htable[2].second = { 0x03 };
    
    htable[3].first = 3;
    htable[3].second = { 0x11, 0x04, 0x00 };
    
    htable[4].first = 3;
    htable[4].second = { 0x05, 0x21, 0x12 };
    
    htable[5].first = 1;
    htable[5].second = { 0x07 };
    
    htable[6].first = 0;
    htable[6].second = { };
    
    htable[7].first = 0;
    htable[7].second = { };
    
    htable[8].first = 0;
    htable[8].second = { };
    
    htable[9].first = 3;
    htable[9].second = { 0xA0, 0xA1, 0xA3 };
    
    htable[10].first = 2;
    htable[10].second = { 0xC3, 0x14 };
    
    htable[11].first = 0;
    htable[11].second = { };
    
    htable[12].first = 1;
    htable[12].second = { 0x27 };
    
    htable[13].first = 0;
    htable[13].second = { };
    
    htable[14].first = 2;
    htable[14].second = { 0x3A, 0x4A };
    
    htable[15].first = 1;
    htable[15].second = { 0x56 };
        
    kpeg::HuffmanTree htree( htable );
    kpeg::inOrder( htree.getTree() );
    
    LOG(kpeg::Logger::Level::DEBUG) << htree.contains( "100" ) << std::endl;
    LOG(kpeg::Logger::Level::DEBUG) << htree.contains( "1100" ) << std::endl;
    LOG(kpeg::Logger::Level::DEBUG) << htree.contains( "101" ) << std::endl;
    LOG(kpeg::Logger::Level::DEBUG) << htree.contains( "1111111111111111" ) << std::endl;
    LOG(kpeg::Logger::Level::DEBUG) << htree.contains( "111010" ) << std::endl;
    LOG(kpeg::Logger::Level::DEBUG) << htree.contains( "111011010000101" ) << std::endl;
    LOG(kpeg::Logger::Level::DEBUG) << htree.contains( "1110110100001100" ) << std::endl;
}

void transformTest()
{
    std::array<std::array<int, 8>, 8> mat 
    {
        std::array<int, 8>{ 52, 55, 61, 66, 70, 61, 64, 73 },
        std::array<int, 8>{ 63, 59, 55, 90, 109, 85, 69, 72 },
        std::array<int, 8>{ 62, 59, 68, 113, 144, 104, 66, 73 },
        std::array<int, 8>{ 63, 58, 71, 122, 154, 106, 70, 69 },
        std::array<int, 8>{ 67, 61, 68, 104, 126, 88, 68, 70 },
        std::array<int, 8>{ 79, 65, 60, 70, 77, 68, 58, 75 },
        std::array<int, 8>{ 85, 71, 64, 59, 55, 61, 65, 83 },
        std::array<int, 8>{ 87, 79, 69, 68, 65, 76, 78, 94 }
    };
    
    std::cout << "Original block:-" << std::endl;
    for (auto&& row: mat )
    {
        for ( auto&& v : row )
            std::cout << v << "\t";
        std::cout << std::endl;
    }
    
    auto coeff = DCTTest( mat );
    
    std::cout << "\nAfter DCT:-" << std::endl;
    
    for (auto&& row: coeff )
    {
        for ( auto&& v : row )
            std::cout << v << "\t";
        std::cout << std::endl;
    }
    
    auto icoeff = IDCTTest( coeff );
    
    std::cout << "\nAfter IDCT:-" << std::endl;
    
    for (auto&& row: icoeff )
    {
        for ( auto&& v : row )
            std::cout << std::roundl( v ) << "\t";
        std::cout << std::endl;
    }
}

std::array<std::array<float, 8>, 8> DCTTest( std::array<std::array<int, 8>, 8>& coeffs )
{
    for ( unsigned v = 0; v < 8; ++v )
    {
        for ( unsigned u = 0; u < 8; ++u )
            coeffs[v][u] -= 128;
    }
    
    std::array<std::array<float, 8>, 8> dct;
    
    for ( unsigned v = 0; v < 8; ++v )
    {
        for ( unsigned u = 0; u < 8; ++u )
        {
            double Cu = u == 0 ? 1.0 / std::sqrt(2) : 1 ;
            double Cv = v == 0 ? 1.0 / std::sqrt(2) : 1 ;
            double coeffY = (0.25) * Cu * Cv;
            
            double s1Y = 0.0;
            
            for ( unsigned y = 0; y < 0 + 8; ++y )
            {
                double s2Y = 0.0;
                
                for ( unsigned x = 0; x < 0 + 8; ++x )
                {
                    s2Y += coeffs[y][x] *
                            std::cos( ( ( 2 * x + 1 ) * u * M_PI ) / 16.0 ) *
                                std::cos( ( ( 2 * y + 1 ) * v * M_PI ) / 16.0 );
                }
                
                s1Y += s2Y;
            }
            
            coeffY *= s1Y;
            
            dct[v][u] = std::roundf( coeffY * 100 ) / 100;
        }
    }
    
    return dct;
}

std::array<std::array<float, 8>, 8> IDCTTest( std::array<std::array<float, 8>, 8>& coeffs )
{
    std::array<std::array<float, 8>, 8> icoeffs;
    
    for ( int x = 0; x < 8; ++x )
    {
        for ( int y = 0; y < 8; ++y )
        {
            double s1 = 0.0;
            
            for ( int u = 0; u < 8; ++u )
            {
                for ( int v = 0; v < 8; ++v )
                {
                    double Cu = u == 0 ? 1.0 / std::sqrt(2.0) : 1.0;
                    double Cv = v == 0 ? 1.0 / std::sqrt(2.0) : 1.0;
                    
                    s1 += Cu * Cv * coeffs[u][v] *std::cos( ( 2 * x + 1 ) * u * M_PI / 16.0 ) *
                                    std::cos( ( 2 * y + 1 ) * v * M_PI / 16.0 );
                }
            }
            
            icoeffs[x][y] = 0.25 * s1;
        }
    }
    
    for ( unsigned v = 0; v < 8; ++v )
    {
        for ( unsigned u = 0; u < 8; ++u )
            icoeffs[v][u] += 128;
    }

    return icoeffs;
}

void colorTest()
{
    float Y = 383;
    float Cb = 128;
    float Cr = 128;
    
    int R = (int)std::floor( Y + 1.402 * ( 1.0 * Cr - 128.0 ) );
    int G = (int)std::floor( Y - 0.344136 * ( 1.0 * Cb - 128.0 ) - 0.714136 * ( 1.0 * Cr - 128.0 ) );
    int B = (int)std::floor( Y + 1.772 * ( 1.0 * Cb - 128.0 ) );
    
    R = std::max( 0, std::min( R, 255 ) );
    G = std::max( 0, std::min( G, 255 ) );
    B = std::max( 0, std::min( B, 255 ) );
            
    std::cout << "(" << R
              << "," << G << ","
              << B << ") "
              << std::endl;
}
