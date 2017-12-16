#include <cmath>
#include <vector>
#include <iomanip>
#include <arpa/inet.h> // htons
#include <utility>

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
        
        // Luminance, DC HT
        m_outputJPEG << JFIF_BYTE_FF << JFIF_DHT;
        m_outputJPEG << (UInt8)0x00 << (UInt8)0x1F; // DHT segment length (including the length bytes)
        // The symbol count for each symbol from 1-bit length to 16-bit length
        m_outputJPEG << (UInt8)0x00 << (UInt8)0x00 << (UInt8)0x01 << (UInt8)0x05 << (UInt8)0x01
                     << (UInt8)0x01 << (UInt8)0x01 << (UInt8)0x01 << (UInt8)0x01 << (UInt8)0x01
                     << (UInt8)0x00 << (UInt8)0x00 << (UInt8)0x00 << (UInt8)0x00 << (UInt8)0x00
                     << (UInt8)0x00 << (UInt8)0x00 << (UInt8)0x00 << (UInt8)0x01 << (UInt8)0x02
                     << (UInt8)0x03 << (UInt8)0x04 << (UInt8)0x05 << (UInt8)0x06 << (UInt8)0x07
                     << (UInt8)0x08 << (UInt8)0x09 << (UInt8)0x0A << (UInt8)0x0B;
        
        // Luminance, AC HT
        m_outputJPEG << JFIF_BYTE_FF << JFIF_DHT;
        m_outputJPEG << (UInt8)0x00 << (UInt8)0xB5; // DHT segment length (including the length bytes)
        m_outputJPEG << (UInt8)0x10 << (UInt8)0x00 << (UInt8)0x02 << (UInt8)0x01 << (UInt8)0x03
                     << (UInt8)0x03 << (UInt8)0x02 << (UInt8)0x04 << (UInt8)0x03 << (UInt8)0x05
                     << (UInt8)0x05 << (UInt8)0x04 << (UInt8)0x04 << (UInt8)0x00 << (UInt8)0x00
                     << (UInt8)0x01 << (UInt8)0x7D << (UInt8)0x01 << (UInt8)0x02 << (UInt8)0x03
                     << (UInt8)0x00 << (UInt8)0x04 << (UInt8)0x11 << (UInt8)0x05 << (UInt8)0x12
                     << (UInt8)0x21 << (UInt8)0x31 << (UInt8)0x41 << (UInt8)0x06 << (UInt8)0x13
                     << (UInt8)0x51 << (UInt8)0x61 << (UInt8)0x07 << (UInt8)0x22 << (UInt8)0x71
                     << (UInt8)0x14 << (UInt8)0x32 << (UInt8)0x81 << (UInt8)0x91 << (UInt8)0xA1
                     << (UInt8)0x08 << (UInt8)0x23 << (UInt8)0x42 << (UInt8)0xB1 << (UInt8)0xC1
                     << (UInt8)0x15 << (UInt8)0x52 << (UInt8)0xD1 << (UInt8)0xF0 << (UInt8)0x24
                     << (UInt8)0x33 << (UInt8)0x62 << (UInt8)0x72 << (UInt8)0x82 << (UInt8)0x09
                     << (UInt8)0x0A << (UInt8)0x16 << (UInt8)0x17 << (UInt8)0x18 << (UInt8)0x19
                     << (UInt8)0x1A << (UInt8)0x25 << (UInt8)0x26 << (UInt8)0x27 << (UInt8)0x28
                     << (UInt8)0x29 << (UInt8)0x2A << (UInt8)0x34 << (UInt8)0x35 << (UInt8)0x36
                     << (UInt8)0x37 << (UInt8)0x38 << (UInt8)0x39 << (UInt8)0x3A << (UInt8)0x43
                     << (UInt8)0x44 << (UInt8)0x45 << (UInt8)0x46 << (UInt8)0x47 << (UInt8)0x48
                     << (UInt8)0x49 << (UInt8)0x4A << (UInt8)0x53 << (UInt8)0x54 << (UInt8)0x55
                     << (UInt8)0x56 << (UInt8)0x57 << (UInt8)0x58 << (UInt8)0x59 << (UInt8)0x5A
                     << (UInt8)0x63 << (UInt8)0x64 << (UInt8)0x65 << (UInt8)0x66 << (UInt8)0x67
                     << (UInt8)0x68 << (UInt8)0x69 << (UInt8)0x6A << (UInt8)0x73 << (UInt8)0x74
                     << (UInt8)0x75 << (UInt8)0x76 << (UInt8)0x77 << (UInt8)0x78 << (UInt8)0x79
                     << (UInt8)0x7A << (UInt8)0x83 << (UInt8)0x84 << (UInt8)0x85 << (UInt8)0x86
                     << (UInt8)0x87 << (UInt8)0x88 << (UInt8)0x89 << (UInt8)0x8A << (UInt8)0x92 << (UInt8)0x93
                     << (UInt8)0x94 << (UInt8)0x95 << (UInt8)0x96 << (UInt8)0x97 << (UInt8)0x98
                     << (UInt8)0x99 << (UInt8)0x9A << (UInt8)0xA2 << (UInt8)0xA3 << (UInt8)0xA4
                     << (UInt8)0xA5 << (UInt8)0xA6 << (UInt8)0xA7 << (UInt8)0xA8 << (UInt8)0xA9
                     << (UInt8)0xAA << (UInt8)0xB2 << (UInt8)0xB3 << (UInt8)0xB4 << (UInt8)0xB5
                     << (UInt8)0xB6 << (UInt8)0xB7 << (UInt8)0xB8 << (UInt8)0xB9 << (UInt8)0xBA
                     << (UInt8)0xC2 << (UInt8)0xC3 << (UInt8)0xC4 << (UInt8)0xC5 << (UInt8)0xC6
                     << (UInt8)0xC7 << (UInt8)0xC8 << (UInt8)0xC9 << (UInt8)0xCA << (UInt8)0xD2
                     << (UInt8)0xD3 << (UInt8)0xD4 << (UInt8)0xD5 << (UInt8)0xD6 << (UInt8)0xD7
                     << (UInt8)0xD8 << (UInt8)0xD9 << (UInt8)0xDA << (UInt8)0xE1 << (UInt8)0xE2
                     << (UInt8)0xE3 << (UInt8)0xE4 << (UInt8)0xE5 << (UInt8)0xE6 << (UInt8)0xE7
                     << (UInt8)0xE8 << (UInt8)0xE9 << (UInt8)0xEA << (UInt8)0xF1 << (UInt8)0xF2
                     << (UInt8)0xF3 << (UInt8)0xF4 << (UInt8)0xF5 << (UInt8)0xF6 << (UInt8)0xF7
                     << (UInt8)0xF8 << (UInt8)0xF9 << (UInt8)0xFA;
        
        // Chrominance, DC HT
        m_outputJPEG << JFIF_BYTE_FF << JFIF_DHT;
        m_outputJPEG << (UInt8)0x00 << (UInt8)0x1F;
        m_outputJPEG << (UInt8)0x01 << (UInt8)0x00 << (UInt8)0x03 << (UInt8)0x01 << (UInt8)0x01
                     << (UInt8)0x01 << (UInt8)0x01 << (UInt8)0x01 << (UInt8)0x01 << (UInt8)0x01
                     << (UInt8)0x01 << (UInt8)0x01 << (UInt8)0x00 << (UInt8)0x00 << (UInt8)0x00
                     << (UInt8)0x00 << (UInt8)0x00 << (UInt8)0x00 << (UInt8)0x01 << (UInt8)0x02
                     << (UInt8)0x03 << (UInt8)0x04 << (UInt8)0x05 << (UInt8)0x06 << (UInt8)0x07
                     << (UInt8)0x08 << (UInt8)0x09 << (UInt8)0x0A << (UInt8)0x0B;
        
        // Chrominance, AC HT
        m_outputJPEG << JFIF_BYTE_FF << JFIF_DHT;
        m_outputJPEG << (UInt8)0x00 << (UInt8)0xB5; // DHT segment length (including the length bytes)
        m_outputJPEG << (UInt8)0x11 << (UInt8)0x00 << (UInt8)0x02 << (UInt8)0x01 << (UInt8)0x02
                     << (UInt8)0x04 << (UInt8)0x04 << (UInt8)0x03 << (UInt8)0x04 << (UInt8)0x07
                     << (UInt8)0x05 << (UInt8)0x04 << (UInt8)0x04 << (UInt8)0x00 << (UInt8)0x01
                     << (UInt8)0x02 << (UInt8)0x77 << (UInt8)0x00 << (UInt8)0x01 << (UInt8)0x02
                     << (UInt8)0x03 << (UInt8)0x11 << (UInt8)0x04 << (UInt8)0x05 << (UInt8)0x21
                     << (UInt8)0x31 << (UInt8)0x06 << (UInt8)0x12 << (UInt8)0x41 << (UInt8)0x51
                     << (UInt8)0x07 << (UInt8)0x61 << (UInt8)0x71 << (UInt8)0x13 << (UInt8)0x22
                     << (UInt8)0x32 << (UInt8)0x81 << (UInt8)0x08 << (UInt8)0x14 << (UInt8)0x42
                     << (UInt8)0x91 << (UInt8)0xA1 << (UInt8)0xB1 << (UInt8)0xC1 << (UInt8)0x09
                     << (UInt8)0x23 << (UInt8)0x33 << (UInt8)0x52 << (UInt8)0xF0 << (UInt8)0x15
                     << (UInt8)0x62 << (UInt8)0x72 << (UInt8)0xD1 << (UInt8)0x0A << (UInt8)0x16
                     << (UInt8)0x24 << (UInt8)0x34 << (UInt8)0xE1 << (UInt8)0x25 << (UInt8)0xF1
                     << (UInt8)0x17 << (UInt8)0x18 << (UInt8)0x19 << (UInt8)0x1A << (UInt8)0x26
                     << (UInt8)0x27 << (UInt8)0x28 << (UInt8)0x29 << (UInt8)0x2A << (UInt8)0x35
                     << (UInt8)0x36 << (UInt8)0x37 << (UInt8)0x38 << (UInt8)0x39 << (UInt8)0x3A
                     << (UInt8)0x43 << (UInt8)0x44 << (UInt8)0x45 << (UInt8)0x46 << (UInt8)0x47
                     << (UInt8)0x48 << (UInt8)0x49 << (UInt8)0x4A << (UInt8)0x53 << (UInt8)0x54
                     << (UInt8)0x55 << (UInt8)0x56 << (UInt8)0x57 << (UInt8)0x58 << (UInt8)0x59
                     << (UInt8)0x5A << (UInt8)0x63 << (UInt8)0x64 << (UInt8)0x65 << (UInt8)0x66
                     << (UInt8)0x67 << (UInt8)0x68 << (UInt8)0x69 << (UInt8)0x6A << (UInt8)0x73
                     << (UInt8)0x74 << (UInt8)0x75 << (UInt8)0x76 << (UInt8)0x77 << (UInt8)0x78
                     << (UInt8)0x79 << (UInt8)0x7A << (UInt8)0x82 << (UInt8)0x83 << (UInt8)0x84
                     << (UInt8)0x85 << (UInt8)0x86 << (UInt8)0x87 << (UInt8)0x88 << (UInt8)0x89 << (UInt8)0x8A
                     << (UInt8)0x92 << (UInt8)0x93 << (UInt8)0x94 << (UInt8)0x95 << (UInt8)0x96
                     << (UInt8)0x97 << (UInt8)0x98 << (UInt8)0x99 << (UInt8)0x9A << (UInt8)0xA2
                     << (UInt8)0xA3 << (UInt8)0xA4 << (UInt8)0xA5 << (UInt8)0xA6 << (UInt8)0xA7
                     << (UInt8)0xA8 << (UInt8)0xA9 << (UInt8)0xAA << (UInt8)0xB2 << (UInt8)0xB3
                     << (UInt8)0xB4 << (UInt8)0xB5 << (UInt8)0xB6 << (UInt8)0xB7 << (UInt8)0xB8
                     << (UInt8)0xB9 << (UInt8)0xBA << (UInt8)0xC2 << (UInt8)0xC3 << (UInt8)0xC4
                     << (UInt8)0xC5 << (UInt8)0xC6 << (UInt8)0xC7 << (UInt8)0xC8 << (UInt8)0xC9
                     << (UInt8)0xCA << (UInt8)0xD2 << (UInt8)0xD3 << (UInt8)0xD4 << (UInt8)0xD5
                     << (UInt8)0xD6 << (UInt8)0xD7 << (UInt8)0xD8 << (UInt8)0xD9 << (UInt8)0xDA
                     << (UInt8)0xE2 << (UInt8)0xE3 << (UInt8)0xE4 << (UInt8)0xE5 << (UInt8)0xE6
                     << (UInt8)0xE7 << (UInt8)0xE8 << (UInt8)0xE9 << (UInt8)0xEA << (UInt8)0xF2
                     << (UInt8)0xF3 << (UInt8)0xF4 << (UInt8)0xF5 << (UInt8)0xF6 << (UInt8)0xF7
                     << (UInt8)0xF8 << (UInt8)0xF9 << (UInt8)0xFA;
        
        ////////////////////////////////////
        // Write start of scan segment
        ////////////////////////////////////
        m_outputJPEG << JFIF_BYTE_FF << JFIF_SOS;
        m_outputJPEG << (UInt8)0x00 << (UInt8)0x0C; // Length of SOS header
        m_outputJPEG << (UInt8)0x03; // # of components
        m_outputJPEG << (UInt8)0x01 << (UInt8)0x00; // HT info for component #1
        m_outputJPEG << (UInt8)0x02 << (UInt8)0x11; // HT info for component #2
        m_outputJPEG << (UInt8)0x03 << (UInt8)0x11; // HT info for component #3
        m_outputJPEG << (UInt8)0x00 << (UInt8)0x3F << (UInt8)0x00; // Skip bytes
        
//         m_outputJPEG << (UInt8)0xF3 << (UInt8)0xFA << (UInt8)0x00
//                      << (UInt8)0xFA << (UInt8)0x02 << (UInt8)0x80 << (UInt8)0x3F;
                     
        auto scanData = generateScanData();
        
        for ( int i = 0; i <= scanData.size() - 8; i += 8 )
        {
            std::bitset<8> word( scanData.substr( i, 8 ) );
            UInt8 w = (UInt8)word.to_ulong();
            
            m_outputJPEG << w;
            if ( w == JFIF_BYTE_FF )
                m_outputJPEG << 0x00;
        }
        
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
    
    const std::string JPEGEncoder::generateScanData()
    {
        std::vector<std::array<std::vector<int>, 3>> MCURle;
        std::string bits;
        
        for ( int iy = 0; iy + 8 - 1 < m_image.getHeight(); iy += 8 )
        {
            for ( int ix = 0; ix + 8 - 1 < m_image.getWidth(); ix += 8 )
            {
                auto rle = generateRLE( iy, ix );
                MCURle.push_back( std::move( rle ) );
            }
        }
        
        // Difference of the current DC coefficient from the previous one.
        // Initially DC difference for first MCU is 0 for each component.
        Int16 DCDiff[3] = { 0,  0, 0 };
        
        // For each MCU, encode it
        for ( int mcu = 0; mcu < MCURle.size(); ++mcu )
        {
            // For each MCU, encode the components separately
            for ( int k = 0; k < 3; ++k )
            {
                // Encode DC coefficient for k-th component
                
                DCDiff[k] = MCURle[mcu][k][1] - DCDiff[k];
                
                Int16 value = DCDiff[k];
                int cat = getValueCategory( value );
                std::string bitRep = valueToBitString( value );
                
                std::string category;
                
                if ( k == YCbCrComponents::Y )
                    category = DC_LUMA_HUFF_CODES[cat];
                else
                    category = DC_CHROMA_HUFF_CODES[cat];
                
                // Add the bits for ( category, bit representation )
                // of the DC coefficient to the scan data bitstream.
                bits += category + bitRep;
                
                // Encode AC coefficients for k-th component
                
                for ( int j = 2; j <= MCURle[mcu][k].size() - 2; j += 2 )
                {
                    // Check EOB
                    if ( MCURle[mcu][k][j] == 0 && MCURle[mcu][k][j + 1] == 0 )
                    {
                        std::string bitRep;
                        
                        if ( k == YCbCrComponents::Y )
                        {
                            bitRep = AC_LUMA_HUFF_CODES[0][0];
                        }
                        else
                        {
                            bitRep = AC_CHROMA_HUFF_CODES[0][0];
                        }
                        
                        bits += bitRep;
                        
                        break;
                    }
                    
                    // Else, determine the zero count and value category
                    // and then add to the scan data bitstream, the huffman
                    // code for the ( zero count, category ) pair followed
                    // by the bitstring for the value.
                    
                    Int16 zeroCount = MCURle[mcu][k][j];
                    Int16 value = MCURle[mcu][k][j + 1];
                    int cat = getValueCategory( value );
                    std::string bitRep = valueToBitString( value );
                    
                    std::string category;
                    
                    if ( k == YCbCrComponents::Y )
                        category = AC_LUMA_HUFF_CODES[zeroCount][cat];
                    else
                        category = AC_CHROMA_HUFF_CODES[zeroCount][cat];
                    
                    bits += category + bitRep;
                }
            }
            
            //std::cout << "Bitstream: " << bits << std::endl;
        }
        
        //std::cout << "Final bitstream: " << bits.size() << std::endl;
        
        if ( bits.size() % 8 != 0 )
        {
            bits.resize( bits.size() + 8 - bits.size() % 8, '1' );
        }
        
        //std::cout << "Final bitstream: " << bits.size() << ", " << bits << std::endl;
        std::cout << "Final bitstream: " << bits << std::endl;
        
        //return std::move( bits );
        
        return bits;
    }
}
