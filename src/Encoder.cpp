#include <cmath>
#include <vector>
#include <iomanip>
#include <arpa/inet.h> // htons

#include "Encoder.hpp"
#include "Logger.hpp"
#include "Markers.hpp"

namespace kpeg
{
    JPEGEncoder::JPEGEncoder()
    {
        LOG(Logger::Level::INFO) << "Created \'JPEGEncoder object\'." << std::endl;
    }
            
    JPEGEncoder::JPEGEncoder( const std::string& filename )
    {
        LOG(Logger::Level::INFO) << "Created \'JPEGEncoder object\'." << std::endl;
    }
    
    JPEGEncoder::~JPEGEncoder()
    {
        
        LOG(Logger::Level::INFO) << "Destroyed \'JPEGEncoder object\'." << std::endl;
    }
    
    bool JPEGEncoder::open( const std::string& filename )
    {
        LOG(Logger::Level::DEBUG) << "Reading input PPM file: \'" + filename + "\'" << std::endl;
        
        if ( !m_image.readRawData( filename ) )
        {
            LOG(Logger::Level::ERROR) << "An error occurred while reading the PPM image file!" << std::endl;
            return false;
        }
        
        LOG(Logger::Level::DEBUG) << "Read input PPM file [OK] \'" + filename + "\'" << std::endl;
        m_filename = filename;
        
        return true;
    }
    
    bool JPEGEncoder::encodeImage()
    {
        LOG(Logger::Level::INFO) << "Encoding PPM image to JPEG..." << std::endl;
        
        transformColorspace();
        levelShiftComponents();
        computeDCT();
        quantize();
        
        if ( !saveToJFIFFile() )
        {
            LOG(Logger::Level::ERROR) << "Encoding incomplete [NOT-OK]" << std::endl;
            return false;
        }
        
        LOG(Logger::Level::INFO) << "Encoding complete [OK]" << std::endl;
        return true;
    }
    
    bool JPEGEncoder::saveToJFIFFile()
    {
        std::string destFile = "output.jpg";
        
        LOG(Logger::Level::INFO) << "Converting & writing JPEG image data to JFIF file: " + destFile + " ..." << std::endl;
        
        m_outputJPEG.open( destFile, std::ios::out | std::ios::binary );
        
        if ( !m_outputJPEG.good() || !m_outputJPEG.is_open() )
        {
            LOG(Logger::Level::ERROR) << "Unable to create destination JFIF file: " << destFile << std::endl;
            return false;
        }
        
        // NOTE: For a detailed description of the markers, see file `Markers.hpp`
        
        m_outputJPEG.unsetf( std::ios::skipws );
        
        ////////////////////////////////////
        // Write start marker
        ////////////////////////////////////
        m_outputJPEG << JFIF_BYTE_FF << JFIF_SOI;
        
        
        ////////////////////////////////////
        // Write application segment 0 marker
        ////////////////////////////////////
        
        // Write marker length identifier
        m_outputJPEG << JFIF_BYTE_FF << JFIF_APP0;
        
        // Write segment length
        m_outputJPEG << (UInt8)0x00 << (UInt8)0x10; // 16 bytes
        
        // Write file identifier mask
        m_outputJPEG << (UInt8)0x4A << (UInt8)0x46 << (UInt8)0x49 << (UInt8)0x46 << (UInt8)0x00; // 'J', 'F', 'I', 'F', '\0'
        
        // Write major & minor version numbers
        // We set version to 1.01
        m_outputJPEG << (UInt8)0x01;
        m_outputJPEG << (UInt8)0x01;
        
        // Write desnity units
        m_outputJPEG << (UInt8)0x01; // We use DPI for denoting pixel density
        
        // Write the X & Y densities
        // We set a DPI of 72 in both X & Y directions
        m_outputJPEG << (UInt8)0x00 << (UInt8)0x48;
        m_outputJPEG << (UInt8)0x00 << (UInt8)0x48;
        
        // Write the thumbnail width & height
        // We don't encode the thumbnail data
        m_outputJPEG << (UInt8)0x00 << (UInt8)0x00;
        
        
        ////////////////////////////////////
        // Write the comment segment
        ////////////////////////////////////
        
        // Write the comment marker
        m_outputJPEG << JFIF_BYTE_FF << JFIF_COM;
        
        std::string comment = "Encoded with libKPEG (https://github.com/TheIllusionistMirage/libKPEG) - Easy to use baseline JPEG library";
        
        // Write the length of the comment segment
        // NOTE: The length includes the two bytes that denote the length
        m_outputJPEG << (UInt8)( (UInt8)( comment.length() >> 8 ) & (UInt8)0xFF00 ); // the first 8 MSBs
        m_outputJPEG << (UInt8)( (UInt8)comment.length() & (UInt8)0x00FF ); // the next 8 LSBs
                
        // Write the comment (only ASCII characters allowed)
        m_outputJPEG << comment;
        
        ////////////////////////////////////
        // Write Quantization Tables
        ////////////////////////////////////
        
        /*
         * Quantization table for luminance (Y) 
         */
        
        // Write DQT marker
        m_outputJPEG << JFIF_BYTE_FF << JFIF_DQT;
        
        // Write the length of the DQT segment
        // NOTE: The length includes the two bytes that denote the length
        m_outputJPEG << (UInt8)0x00 << (UInt8)0x43;
        
        // Write quantization table info
        // NOTE: Bits 7-4 denote QT#, bits 3-0 denote QT precision
        // libKPEG supports only 8-bit JPEG images, so bits 7-4 are 0
        m_outputJPEG << (UInt8)0x00;
        
        // Write the 64 entries of the QT in zig-zag order
        for ( int i = 0; i < 64; ++i )
        {
            auto index = zzOrderToMatIndices( i );
            m_outputJPEG << (UInt8)M_QT_MAT_LUMA[index.first][index.second];
        }
        
        /**
         * Quantization table for chrominance (Cb & Cr)
         */
        
        // Write DQT marker
        m_outputJPEG << JFIF_BYTE_FF << JFIF_DQT;
        
        // Write the length of the DQT segment
        // NOTE: The length includes the two bytes that denote the length
        m_outputJPEG << (UInt8)0x00 << (UInt8)0x43;
        
        // Write quantization table info
        // NOTE: Bits 7-4 denote QT#, bits 3-0 denote QT precision
        // libKPEG supports only 8-bit JPEG images, so bits 7-4 are 0
        m_outputJPEG << (UInt8)0x01;
        
        // Write the 64 entries of the QT in zig-zag order
        for ( int i = 0; i < 64; ++i )
        {
            auto index = zzOrderToMatIndices( i );
            m_outputJPEG << (UInt8)M_QT_MAT_CHROMA[index.first][index.second];
        }
        
        
        ////////////////////////////////////
        // Write start of frame 0 segment
        ////////////////////////////////////
        
        // Write SOF-0 marker identifier
        m_outputJPEG << JFIF_BYTE_FF << JFIF_SOF0;
        
        // Write SOF-0 segment length
        m_outputJPEG << (UInt8)0x00 << (UInt8)0x11; // 8 + 3 * 3
        
        // Write data precision
        m_outputJPEG << (UInt8)0x08;
        
        // Write image dimensions
        
        // Height
        m_outputJPEG << (UInt8)( (UInt8)( m_image.getHeight() >> 8 ) & (UInt8)0xFF00 ); // the first 8 MSBs
        m_outputJPEG << (UInt8)( (UInt8)m_image.getHeight() & (UInt8)0x00FF ); // the next 8 LSBs
        
        // Width
        m_outputJPEG << (UInt8)( (UInt8)( m_image.getWidth() >> 8 ) & (UInt8)0xFF00 ); // the first 8 MSBs
        m_outputJPEG << (UInt8)( (UInt8)m_image.getWidth() & (UInt8)0x00FF ); // the next 8 LSBs
        
        // Write the number of components
        // NOTE: For now, libKPEG doesn't actually remove the chroma components; they're just set to all 0s
        m_outputJPEG << (UInt8)0x03;
        
        // Write component info for each of the components (each component takes 3 bytes)
        
        // Luminance (Y)
        m_outputJPEG << (UInt8)0x01; // Component ID (Y=1, Cb=2, Cr=3)
        m_outputJPEG << (UInt8)0x11; // Sampling factors (Bits 7-4: Horizontal, Bits 3-0: Vertical)
        m_outputJPEG << (UInt8)0x00; // Quantization table #
        
        // Chrominance (Cb)
        m_outputJPEG << (UInt8)0x02; // Component ID (Y=1, Cb=2, Cr=3)
        m_outputJPEG << (UInt8)0x11; // Sampling factors (Bits 7-4: Horizontal, Bits 3-0: Vertical)
        m_outputJPEG << (UInt8)0x01; // Quantization table #
        
        // Chrominance (Cr)
        m_outputJPEG << (UInt8)0x03; // Component ID (Y=1, Cb=2, Cr=3)
        m_outputJPEG << (UInt8)0x11; // Sampling factors (Bits 7-4: Horizontal, Bits 3-0: Vertical)
        m_outputJPEG << (UInt8)0x01; // Quantization table #
        
        ////////////////////////////////////
        // Write DHT segments
        ////////////////////////////////////
        m_outputJPEG << JFIF_BYTE_FF << JFIF_DHT;
        
        ////////////////////////////////////
        // Write start of scan segment
        ////////////////////////////////////
        m_outputJPEG << JFIF_BYTE_FF << JFIF_SOS;
        
        ////////////////////////////////////
        // Write end marker
        ////////////////////////////////////
        m_outputJPEG << JFIF_BYTE_FF << JFIF_EOI;
        
        LOG(Logger::Level::INFO) << "Finished writing JPEG image data to JFIF file: " + destFile + " [OK]" << std::endl;
        return true;
    }
    
    void JPEGEncoder::transformColorspace()
    {
        LOG(Logger::Level::INFO) << "Performing colorspace transformation from R-G-B to Y-Cb-Cr..." << std::endl;
        
        for ( int y = 0; y < m_image.getHeight(); ++y )
        {
            for ( int x = 0; x < m_image.getWidth(); ++x )
            {
                int R = int( (*m_image.getFlPixelPtr())[y][x].comp[RGBComponents::RED] );
                int G = int( (*m_image.getFlPixelPtr())[y][x].comp[RGBComponents::GREEN] );
                int B = int( (*m_image.getFlPixelPtr())[y][x].comp[RGBComponents::BLUE] );
                
                UInt8 Y = std::floor( 0.299f * R + 0.587f * G + 0.114f * B );
                UInt8 Cb = std::floor( - 0.1687f * R - 0.3313f * G + 0.5f * B + 128.f );
                UInt8 Cr = std::floor( 0.5f * R - 0.4187f * G - 0.0813f * B + 128.f );
                
                (*m_image.getFlPixelPtr())[y][x].comp[YCbCrComponents::Y] = Y;
                (*m_image.getFlPixelPtr())[y][x].comp[YCbCrComponents::Cb] = Cb;
                (*m_image.getFlPixelPtr())[y][x].comp[YCbCrComponents::Cr] = Cr;
            }
        }
        
//         for ( auto&& row : *m_image.getFlPixelPtr() )
//         {
//             for ( auto&& val : row )
//             {
//                 std::cout << val.comp[0] << "," << val.comp[1] << "," << val.comp[2] << "\t";
//             }
//             std::cout << std::endl;
//         }
//         std::cout << std::endl;
        
        LOG(Logger::Level::INFO) << "Colorspace transformation complete [OK]" << std::endl;
    }
    
    void JPEGEncoder::levelShiftComponents()
    {
        LOG(Logger::Level::INFO) << "Performing level shift on components..." << std::endl;
        
        for ( int y = 0; y < m_image.getHeight(); ++y )
        {
            for ( int x = 0; x < m_image.getWidth(); ++x )
            {
                (*m_image.getFlPixelPtr())[y][x].comp[YCbCrComponents::Y] -= 128;
                (*m_image.getFlPixelPtr())[y][x].comp[YCbCrComponents::Cb] -= 128;
                (*m_image.getFlPixelPtr())[y][x].comp[YCbCrComponents::Cr] -= 128;
            }
        }
        
        LOG(Logger::Level::INFO) << "Level shift complete [OK]" << std::endl;
        
//         for ( auto&& row : *m_image.getFlPixelPtr() )
//         {
//             for ( auto&& val : row )
//             {
//                 std::cout << val.comp[0] << "," << val.comp[1] << "," << val.comp[2] << "\t";
//             }
//             std::cout << std::endl;
//         }
//         std::cout << std::endl;
    }
    
    void kpeg::JPEGEncoder::computeDCT()
    {
        LOG(Logger::Level::INFO) << "Applying Forward DCT on components..." << std::endl;
        
        // Traverse the pixel pointer, 8x8 blocks at a time
        for ( int iy = 0; iy + 8 - 1 < m_image.getHeight(); iy += 8 )
        {
            for ( int ix = 0; ix + 8 - 1 < m_image.getWidth(); ix += 8 )
            {
                std::array<std::array<FPixel, 8>, 8> coefficients;
                
                for ( int v = 0; v < 8; ++v )
                {
                    for ( int u = 0; u < 8; ++u )
                    {
                        float Cu = u == 0 ? 1.0 / std::sqrt(2.f) : 1.f;
                        float Cv = v == 0 ? 1.0 / std::sqrt(2.f) : 1.f;
                        float coeff[3] = { 0.f, 0.f, 0.f };
                        
                        for ( int y = iy; y < iy + 8; ++y )
                        {
                            for ( int x = ix; x < ix + 8; ++x )
                            {
                                for ( int c = 0; c < 3; ++c )
                                {
                                    coeff[c] += (*m_image.getFlPixelPtr())[y][x].comp[c] *
                                                 std::cos( ( 2 * x + 1 ) * u * M_PI / 16 ) *
                                                  std::cos( ( 2 * y + 1 ) * v * M_PI / 16 );
                                }
                            }
                        }
                        
                        coeff[YCbCrComponents::Y] = 0.25f * Cu * Cv * coeff[0];
                        coeff[YCbCrComponents::Cb] = 0.25f * Cu * Cv * coeff[1];
                        coeff[YCbCrComponents::Cr] = 0.25f * Cu * Cv * coeff[2];
                        
                        coefficients[v][u].comp[YCbCrComponents::Y] = std::roundf( coeff[YCbCrComponents::Y] * 100 ) / 100;
                        coefficients[v][u].comp[YCbCrComponents::Cb] = std::roundf( coeff[YCbCrComponents::Cb] * 100 ) / 100;
                        coefficients[v][u].comp[YCbCrComponents::Cr] = std::roundf( coeff[YCbCrComponents::Cr] * 100 ) / 100;
                    }
                }
                
                for ( int y = iy; y < iy + 8; ++y )
                {
                    for ( int x = ix; x < ix + 8; ++x )
                    {
                        (*m_image.getFlPixelPtr())[y][x].comp[YCbCrComponents::Y] = coefficients[y - iy][x - ix].comp[YCbCrComponents::Y];
                        (*m_image.getFlPixelPtr())[y][x].comp[YCbCrComponents::Cb] = coefficients[y - iy][x - ix].comp[YCbCrComponents::Cb];
                        (*m_image.getFlPixelPtr())[y][x].comp[YCbCrComponents::Cr] = coefficients[y - iy][x - ix].comp[YCbCrComponents::Cr];
                    }
                }
            }
        }
        
//         for ( auto&& row : *m_image.getFlPixelPtr() )
//         {
//             for ( auto&& val : row )
//             {
//                 std::cout << val.comp[0] << "," << val.comp[1] << "," << val.comp[2] << "\t";
//             }
//             std::cout << std::endl;
//         }
//         std::cout << std::endl;
        
        LOG(Logger::Level::INFO) << "Forward DCT applied [OK]" << std::endl;
    }
    
    void JPEGEncoder::quantize()
    {
        LOG(Logger::Level::INFO) << "Quantizing components..." << std::endl;
        
        for ( int iy = 0; iy + 8 - 1 < m_image.getHeight(); iy += 8 )
        {
            for ( int ix = 0; ix + 8 - 1 < m_image.getWidth(); ix += 8 )
            {
                for ( int v = 0; v < 8; ++v )
                {
                    for ( int u = 0; u < 8; ++u )
                    {
                        (*m_image.getFlPixelPtr())[iy + v][ix + u].comp[YCbCrComponents::Y] /= M_QT_MAT_LUMA[v][u];
                        (*m_image.getFlPixelPtr())[iy + v][ix + u].comp[YCbCrComponents::Cb] /= M_QT_MAT_CHROMA[v][u];
                        (*m_image.getFlPixelPtr())[iy + v][ix + u].comp[YCbCrComponents::Cr] /= M_QT_MAT_CHROMA[v][u];
                        
                        (*m_image.getFlPixelPtr())[iy + v][ix + u].comp[YCbCrComponents::Y] = std::round( (*m_image.getFlPixelPtr())[iy + v][ix + u].comp[YCbCrComponents::Y] );
                        (*m_image.getFlPixelPtr())[iy + v][ix + u].comp[YCbCrComponents::Cb] = std::round( (*m_image.getFlPixelPtr())[iy + v][ix + u].comp[YCbCrComponents::Cb] );
                        (*m_image.getFlPixelPtr())[iy + v][ix + u].comp[YCbCrComponents::Cr] = std::round( (*m_image.getFlPixelPtr())[iy + v][ix + u].comp[YCbCrComponents::Cr] );
                    }
                }
            }
        }
        
//         for ( auto&& row : *m_image.getFlPixelPtr() )
//         {
//             for ( auto&& val : row )
//             {
//                 std::cout << val.comp[0] << "," << val.comp[1] << "," << val.comp[2] << "\t";
//             }
//             std::cout << std::endl;
//         }
//         std::cout << std::endl;
        
        LOG(Logger::Level::INFO) << "Quantization complete [OK]" << std::endl;
    }
    
    std::array<std::vector<int>, 3> JPEGEncoder::generateRLE( const int y, const int x )
    {
        LOG(Logger::Level::DEBUG) << "Generating Zero Run-Length Coding for MCU: " << y / 8 << "," << x / 8 << "..." << std::endl;
        
        std::array< std::array<int, 64>, 3 > ZZ;
        
        for ( int c = 0; c < 3; ++c )
        {
            for ( int i = 0; i < 64; ++i )
            {
                auto index = zzOrderToMatIndices( i );
                ZZ[c][i] = (*m_image.getFlPixelPtr())[index.first + y][index.second + x].comp[c];
            }
        }
        
//         for ( auto&& comp : ZZ )
//         {
//             for ( auto&& val : comp )
//                 std::cout << val << " ";
//             std::cout << std::endl;
//         }
//         std::cout << std::endl;
        
        std::array<std::vector<int>, 3> ZRLE;
        
        for ( int c = 0; c < 3; ++c )
        {
            int zeroCount = 0;
            int i = 0;
            
            while ( i < 64 )
            {
                zeroCount = 0;
                int j = i;
                
                while ( j < 64 && ZZ[c][j] == 0 )
                {
                    zeroCount++;
                    j++;
                }
                
                if ( j == 64 )
                {
                    //j--;
                    ZRLE[c].push_back( 0 );
                    ZRLE[c].push_back( 0 );
                    break;
                }
                
                if ( zeroCount < 16 )
                {
                    ZRLE[c].push_back( zeroCount );
                    ZRLE[c].push_back( ZZ[c][j] );
                }
                
                else
                {
                    while ( zeroCount >= 16 )
                    {
                        ZRLE[c].push_back( 15 );
                        ZRLE[c].push_back( 0 );
                        
                        zeroCount -= 16;
                    }
                    
                    ZRLE[c].push_back( zeroCount );
                    ZRLE[c].push_back( ZZ[c][j] );
                }
                
                i = j + 1;
            }
            
//             ZRLE[c].push_back( 0 );
//             ZRLE[c].push_back( 0 );
        }
        
        LOG(Logger::Level::DEBUG) << "Finished generating Zero Run-Length Coding for MCU: " << y / 8 << "," << x / 8 << " [OK]" << std::endl;
        
//         for ( auto&& comp : ZRLE )
//         {
//             for ( auto&& val : comp )
//                 std::cout << val << " ";
//             std::cout << std::endl;
//         }
        
        return ZRLE;
    }
}
