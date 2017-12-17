#include <arpa/inet.h> // htons
#include <iomanip>
#include <sstream>

#include "Decoder.hpp"
#include "Logger.hpp"
#include "Markers.hpp"
#include "Utility.hpp"

namespace kpeg
{
    JPEGDecoder::JPEGDecoder() //:
     //m_huffTableCount(0)
    {
        LOG(Logger::Level::INFO) << "Created \'JPEGDecoder object\'." << std::endl;
    }
            
    JPEGDecoder::JPEGDecoder( const std::string& filename ) //:
     //m_huffTableCount(0)
    {
        LOG(Logger::Level::INFO) << "Created \'JPEGDecoder object\'." << std::endl;
    }
    
    JPEGDecoder::~JPEGDecoder()
    {
        close();
        LOG(Logger::Level::INFO) << "Destroyed \'JPEGDecoder object\'." << std::endl;
    }
    
    bool JPEGDecoder::open( const std::string& filename )
    {
        m_imageFile.open( filename, std::ios::in | std::ios::binary );
        
        if ( !m_imageFile.is_open() || !m_imageFile.good() )
        {
            LOG(Logger::Level::ERROR) << "Unable to open image: \'" + filename + "\'" << std::endl;
            return false;
        }
        
        LOG(Logger::Level::INFO) << "Opened JPEG image: \'" + filename + "\'" << std::endl;
        
        m_filename = filename;
        
        return true;
    }
    
    void JPEGDecoder::close()
    {
        m_imageFile.close();
        LOG(Logger::Level::INFO) << "Closed image file: \'" + m_filename + "\'" << std::endl;
    }
    
    JPEGDecoder::ResultCode JPEGDecoder::parseSegmentInfo( const UInt8 byte )
    {
        if ( byte == JFIF_BYTE_0 || byte == JFIF_BYTE_FF )
            return ERROR;
        
        switch( byte )
        {
            case JFIF_SOI       :   LOG(Logger::Level::INFO) << "Found segment, Start of Image (FFD8)" << std::endl; return ResultCode::SUCCESS;
            case JFIF_APP0      :   LOG(Logger::Level::INFO) << "Found segment, JPEG/JFIF Image Marker segment (APP0)" << std::endl; parseJFIFSegment(); return ResultCode::SUCCESS;
            case JFIF_COM       :   LOG(Logger::Level::INFO) << "Found segment, Comment(FFFE)" << std::endl; parseComment(); return ResultCode::SUCCESS;
            case JFIF_DQT       :   LOG(Logger::Level::INFO) << "Found segment, Define Quantization Table (FFDB)" << std::endl; parseQuantizationTable(); return ResultCode::SUCCESS;
            case JFIF_SOF0      :   LOG(Logger::Level::INFO) << "Found segment, Start of Frame 0: Baseline DCT (FFC0)" << std::endl; return parseSOF0Segment(); //return ResultCode::SUCCESS;
            case JFIF_SOF1      :   LOG(Logger::Level::INFO) << "Found segment, Start of Frame 1: Extended Sequential DCT (FFC1), Not supported" << std::endl; return ResultCode::TERMINATE;
             case JFIF_SOF2      :   LOG(Logger::Level::INFO) << "Found segment, Start of Frame 2: Progressive DCT (FFC2), Not supported" << std::endl; return ResultCode::TERMINATE;
//             case JFIF_SOF2      :   LOG(Logger::Level::INFO) << "Found segment, Start of Frame 2: Progressive DCT (FFC2)" << std::endl; parseSOF0Segment(); return ResultCode::SUCCESS;
            case JFIF_DHT       :   LOG(Logger::Level::INFO) << "Found segment, Define Huffman Table (FFC4)" << std::endl; parseHuffmanTable(); return ResultCode::SUCCESS;
            case JFIF_SOS       :   LOG(Logger::Level::INFO) << "Found segment, Start of Scan (FFDA)" << std::endl; parseSOSSegment(); return ResultCode::SUCCESS;
//             case JFIF_EOI       :   LOG(Logger::Level::INFO) << "Found segment, End of Image (FFD9)" << std::endl;  return ResultCode::SUCCESS;
        }
        
        //return ResultCode::DECODE_INCOMPLETE;
        return ResultCode::SUCCESS;
    }
    
    bool JPEGDecoder::dumpRawData()
    {
        std::size_t extPos = m_filename.find( ".jpg" );
        
        if ( extPos == std::string::npos )
            extPos = m_filename.find( ".jpeg" );
        
        std::string targetFilename = m_filename.substr( 0, extPos ) + ".ppm";
        m_image.dumpRawData( targetFilename );
        
        return true;
    }
    
    JPEGDecoder::ResultCode JPEGDecoder::decodeImageFile()
    {
        if ( !m_imageFile.is_open() || !m_imageFile.good() )
        {
            LOG(Logger::Level::ERROR) << "Unable scan image file: \'" + m_filename + "\'" << std::endl;
            return ResultCode::ERROR;
        }
        
        LOG(Logger::Level::INFO) << "Started decoding process..." << std::endl;
        
        UInt8 byte;
        ResultCode status = ResultCode::DECODE_DONE;
        
        // TODO: Fix this spaghetti code for keeping track of decode status
        
        while ( m_imageFile >> std::noskipws >> byte )
        {
            if ( byte == JFIF_BYTE_FF )
            {
                m_imageFile >> std::noskipws >> byte;
                
                ResultCode code = parseSegmentInfo(byte);
                
                if ( code == ResultCode::SUCCESS )
                    continue;
                else if ( code == ResultCode::TERMINATE )
                {
                    status = ResultCode::TERMINATE;
                    break;
                }
                else if ( code == ResultCode::DECODE_INCOMPLETE )
                {
                    status = ResultCode::DECODE_INCOMPLETE;
                    break;
                }
            }
            else
            {
                //std::cout <<m_imageFile.tellg() << "ZZZZZZ: " << std::hex << byte << std::endl;
                LOG(Logger::Level::ERROR) << "[ FATAL ] Invalid JFIF file! Terminating..." << std::endl;
                status = ResultCode::ERROR;
                break;
            }
        }
        
        if ( status == ResultCode::DECODE_DONE )
        {
            decodeScanData();
            m_image.createImageFromMCUs( m_MCU );
            LOG(Logger::Level::INFO) << "Finished decoding process [OK]." << std::endl;
        }
        else if ( status == ResultCode::TERMINATE )
        {
            LOG(Logger::Level::INFO) << "Terminated decoding process [NOT-OK]." << std::endl;
        }
        
        else if ( status == ResultCode::DECODE_INCOMPLETE )
        {
            LOG(Logger::Level::INFO) << "Decoding process incomplete [NOT-OK]." << std::endl;
        }
        
        return status;
    }
    
//     void JPEGDecoder::displayImage()
//     {
//         LOG(Logger::Level::INFO) << "Displaying decoded JPEG image: \'" + m_filename + "\'" << std::endl;
//         
//         m_imageViewer.setImagePtr( m_image.getPixelPtr() );
//         m_imageViewer.draw();
//         
//         LOG(Logger::Level::INFO) << "Finished displaying decoded image [OK]]" << std::endl;
//     }
    
    void JPEGDecoder::parseJFIFSegment()
    {
        if ( !m_imageFile.is_open() || !m_imageFile.good() )
        {
            LOG(Logger::Level::ERROR) << "Unable scan image file: \'" + m_filename + "\'" << std::endl;
            return;
        }
        
        LOG(Logger::Level::DEBUG) << "Parsing JPEG/JFIF marker segment (APP-0)..." << std::endl;
        
        UInt16 lenByte = 0;
        UInt8 byte = 0;
        
        m_imageFile.read( reinterpret_cast<char *>( &lenByte ), 2 );
        lenByte = htons( lenByte );
        std::size_t curPos = m_imageFile.tellg();
        
        LOG(Logger::Level::DEBUG) << "JFIF Application marker segment length: " << lenByte << std::endl;
        
        // Skip the 'JFIF\0' bytes
        m_imageFile.seekg( 5, std::ios_base::cur );
        
        UInt8 majVersionByte, minVersionByte;
        m_imageFile >> std::noskipws >> majVersionByte >> minVersionByte;
        
        LOG(Logger::Level::DEBUG) << "JFIF version: " << (int)majVersionByte << "." << (int)(minVersionByte >> 4) << (int)(minVersionByte & 0x0F) << std::endl;
        
        std::string majorVersion = std::to_string( majVersionByte );
        std::string minorVersion = std::to_string( (int)(minVersionByte >> 4) );
        minorVersion +=  std::to_string( (int)(minVersionByte & 0x0F) );
        
        m_image.setJPEGVersion( std::string( majorVersion + "." + minorVersion ) );
        
        UInt8 densityByte;
        m_imageFile >> std::noskipws >> densityByte;
        
        std::string densityUnit = "";
        switch( densityByte )
        {
            case 0x00: densityUnit = "Pixel Aspect Ratio"; break;
            case 0x01: densityUnit = "Pixels per inch (DPI)"; break;
            case 0x02: densityUnit = "Pixels per centimeter"; break;
        }
        
        LOG(Logger::Level::DEBUG) << "Image density unit: " << densityUnit << std::endl;
        
        UInt16 xDensity = 0, yDensity = 0;
        
        m_imageFile.read( reinterpret_cast<char *>( &xDensity ), 2 );
        m_imageFile.read( reinterpret_cast<char *>( &yDensity ), 2 );
        
        xDensity = htons(xDensity);
        yDensity = htons(yDensity);
        
        LOG(Logger::Level::DEBUG) << "Horizontal image density: " << xDensity << std::endl;
        LOG(Logger::Level::DEBUG) << "Vertical image density: " << yDensity << std::endl;
        
        // Ignore the image thumbnail data
        UInt8 xThumb = 0, yThumb = 0;
        m_imageFile >> std::noskipws >> xThumb >> yThumb;        
        m_imageFile.seekg( 3 * xThumb * yThumb, std::ios_base::cur );
        
        LOG(Logger::Level::DEBUG) << "Finished parsing JPEG/JFIF marker segment (APP-0) [OK]" << std::endl;
        //std::cout << "Current file pos: " << m_imageFile.tellg() << std::endl;
    }
    
    void JPEGDecoder::parseQuantizationTable()
    {
        if ( !m_imageFile.is_open() || !m_imageFile.good() )
        {
            LOG(Logger::Level::ERROR) << "Unable scan image file: \'" + m_filename + "\'" << std::endl;
            return;
        }
        
        LOG(Logger::Level::DEBUG) << "Parsing quantization table segment..." << std::endl;
        
        UInt16 lenByte = 0;
        UInt8 PqTq;
        UInt8 Qi;
        //int QTNumber = 0;
        
        m_imageFile.read( reinterpret_cast<char *>( &lenByte ), 2 );
        lenByte = htons( lenByte );
        LOG(Logger::Level::DEBUG) << "Quantization table segment length: " << (int)lenByte << std::endl;
        
        lenByte -= 2;
        
        //std::cout << "ZZZZ: " << int(lenByte) / 65 << std::endl;
        
        for ( int qt = 0; qt < int(lenByte) / 65; ++qt )
        {
            m_imageFile >> std::noskipws >> PqTq;
            
            int precision = PqTq >> 4; // Precision is always 8-bit for baseline DCT
            int QTtable = PqTq & 0x0F; // Quantization table number (0-3)
            
            LOG(Logger::Level::DEBUG) << "Quantization Table Number: " << QTtable << std::endl;
            LOG(Logger::Level::DEBUG) << "Quantization Table #" << QTtable << " precision: " << (precision == 0 ? "8-bit" : "16-bit" ) << std::endl;
            
            m_QTables.push_back({});
            
            // Populate quantization table #QTtable
            
            for ( auto i = 0; i < 64; ++i )
            {
                m_imageFile >> std::noskipws >> Qi;
                
//                 if ( Qi == JFIF_BYTE_FF )
//                 {
//                     LOG(Logger::Level::ERROR) << "Unexpected start of marker at offest: " << (int)m_imageFile.tellg() - 1 << std::endl;
//                     return;
//                 }
                
                m_QTables[QTtable].push_back( (UInt16)Qi );
            }
        }
        
//         int qt[8][8];
//         for ( auto i = 0; i < m_QTables[QTtable].size(); ++i )
//         {
//             auto y = zzOrderToMatIndices( i ).first;
//             auto x = zzOrderToMatIndices( i ).second;            
//             qt[y][x] = m_QTables[QTtable][i];
//             //std::cout << i << std::endl;
//         }
//         
//         for ( auto i = 0; i < 8; ++i)
//         {
//             for ( auto j = 0; j < 8; ++j)
//                 std::cout << qt[i][j] << " ";
//             std::cout << std::endl;
//         }
        
        LOG(Logger::Level::DEBUG) << "Finished parsing quantization table segment [OK]" << std::endl;
//         printCurrPos();
    }
    
    JPEGDecoder::ResultCode JPEGDecoder::parseSOF0Segment()
    {
        if ( !m_imageFile.is_open() || !m_imageFile.good() )
        {
            LOG(Logger::Level::ERROR) << "Unable scan image file: \'" + m_filename + "\'" << std::endl;
            return ResultCode::ERROR;
        }
        
        LOG(Logger::Level::DEBUG) << "Parsing SOF-0 segment..." << std::endl;
        
        UInt16 lenByte, imgHeight, imgWidth;
        UInt8 precision, compCount;
        
        m_imageFile.read( reinterpret_cast<char *>( &lenByte ), 2 );
        lenByte = htons( lenByte );
        
        LOG(Logger::Level::DEBUG) << "SOF-0 segment length: " << (int)lenByte << std::endl;
        
        m_imageFile >> std::noskipws >> precision;
        LOG(Logger::Level::DEBUG) << "SOF-0 segment data precision: " << (int)precision << std::endl;
        
        m_imageFile.read( reinterpret_cast<char *>( &imgHeight ), 2 );
        m_imageFile.read( reinterpret_cast<char *>( &imgWidth ), 2 );
        
        imgHeight = htons( imgHeight );
        imgWidth = htons( imgWidth );
        
        LOG(Logger::Level::DEBUG) << "Image height: " << (int)imgHeight << std::endl;
        LOG(Logger::Level::DEBUG) << "Image width: " << (int)imgWidth << std::endl;
        
        m_imageFile >> std::noskipws >> compCount;
        
        LOG(Logger::Level::DEBUG) << "No. of components: " << (int)compCount << std::endl;
        
        UInt8 compID = 0, sampFactor = 0, QTNo = 0;
        
        bool isNonSampled = true;
        
        for ( auto i = 0; i < 3; ++i )
        {
            m_imageFile >> std::noskipws >> compID >> sampFactor >> QTNo;
            
            LOG(Logger::Level::DEBUG) << "Component ID: " << (int)compID << std::endl;
            LOG(Logger::Level::DEBUG) << "Sampling Factor, Horizontal: " << int( sampFactor >> 4 ) << ", Vertical: " << int( sampFactor & 0x0F ) << std::endl;
            LOG(Logger::Level::DEBUG) << "Quantization table no.: " << (int)QTNo << std::endl;
            
            if ( ( sampFactor >> 4 ) != 1 || ( sampFactor & 0x0F ) != 1 )
                isNonSampled = false;
        }
        
        if ( !isNonSampled )
        {
            LOG(Logger::Level::INFO) << "Chroma subsampling not yet supported!" << std::endl;
            LOG(Logger::Level::DEBUG) << "Chroma subsampling is not 4:4:4, terminating..." << std::endl;
            return ResultCode::TERMINATE;
        }
        
        LOG(Logger::Level::DEBUG) << "Finished parsing SOF-0 segment [OK]" << std::endl;
        //printCurrPos();
        
        m_image.setDimensions( imgWidth, imgHeight );
        
        return ResultCode::SUCCESS;
    }
    
    void JPEGDecoder::parseHuffmanTable()
    {
        if ( !m_imageFile.is_open() || !m_imageFile.good() )
        {
            LOG(Logger::Level::ERROR) << "Unable scan image file: \'" + m_filename + "\'" << std::endl;
            return;
        }
        
        LOG(Logger::Level::DEBUG) << "Parsing Huffman table segment..." << std::endl;
        
        UInt16 len;
        m_imageFile.read( reinterpret_cast<char *>( &len ), 2 );
        len = htons( len );
        
        LOG(Logger::Level::DEBUG) << "Huffman table length: " << (int)len << std::endl;
        
        int segmentEnd = (int)m_imageFile.tellg() + len - 2;
        //len -= 2;
        
        while ( m_imageFile.tellg() < segmentEnd )
        {
            UInt8 htinfo;
            m_imageFile >> std::noskipws >> htinfo;
            
            int HTType = int( (htinfo & 0x10) >> 4 );
            int HTNumber = int(htinfo & 0x0F);
            
            LOG(Logger::Level::DEBUG) << "Huffman table type: " << HTType << std::endl;
            LOG(Logger::Level::DEBUG) << "Huffman table #: " << HTNumber << std::endl;
            
            int totalSymbolCount = 0;
            UInt8 symbolCount;
            
            //LOG(Logger::Level::DEBUG) << "Displaying Huffman table, (Type: " << HTType << ", #: " << HTNumber << ") symbol counts... " << std::endl;
            for ( auto i = 1; i <= 16; ++i )
            {
                m_imageFile >> std::noskipws >> symbolCount;
    //             LOG(Logger::Level::DEBUG) << "Huffman table, (Type:" << HTType << ",#:" << HTNumber << "), Length=" << i << " : " << (int)symbolCount << std::endl;
                //LOG(Logger::Level::DEBUG) << "Code length=" << i << " : " << (int)symbolCount << std::endl;
                
                m_huffmanTable[HTType][HTNumber][i-1].first = (int)symbolCount;
                totalSymbolCount += (int)symbolCount;
            }
            
            int syms = 0;
            for ( auto i = 0; syms < totalSymbolCount;  )
            {
                UInt8 code;
                m_imageFile >> std::noskipws >> code;
                //LOG(Logger::Level::DEBUG) << "Huffman code: 0x" << std::hex << std::setfill('0') << std::setw(2) << std::setprecision(8) << (int)code << "(" << std::bitset<8>(int(code)) << ")" << std::endl;
                
                if ( m_huffmanTable[HTType][HTNumber][i].first == 0 )
                {
                    while ( m_huffmanTable[HTType][HTNumber][++i].first == 0 );
                }
                
                m_huffmanTable[HTType][HTNumber][i].second.push_back( code );
                syms++;
                
                if ( m_huffmanTable[HTType][HTNumber][i].first == m_huffmanTable[HTType][HTNumber][i].second.size() )
                    i++;
            }
            
            LOG(Logger::Level::DEBUG) << "Printing symbols for Huffman table (" << HTType << "," << HTNumber << ")..." << std::endl;
            
            int totalCodes = 0;
            for ( auto i = 0; i < 16; ++i )
            {
                std::string codeStr = "";
                for ( auto&& symbol : m_huffmanTable[HTType][HTNumber][i].second )
                {
                    std::stringstream ss;
                    ss << "0x" << std::hex << std::setfill('0') << std::setw(2) << std::setprecision(16) << (int)symbol;
                    codeStr += ss.str() + " ";
                    totalCodes++;
                }
                
                LOG(Logger::Level::DEBUG) << "Code length: " << i+1 // m_huffmanTable[HTType][HTNumber][i].first
                                        << ", Symbol count: " << m_huffmanTable[HTType][HTNumber][i].second.size()
                                        << ", Symbols: " << codeStr << std::endl;
            }
            
            LOG(Logger::Level::DEBUG) << "Total Huffman codes for Huffman table(Type:" << HTType << ",#:" << HTNumber << "): " << totalCodes << std::endl;
            
            m_huffmanTree[HTType][HTNumber].constructHuffmanTree( m_huffmanTable[HTType][HTNumber] );
            auto htree = m_huffmanTree[HTType][HTNumber].getTree();
            LOG(Logger::Level::DEBUG) << "Huffman codes:-" << std::endl;
            inOrder( htree );
        }
        
        LOG(Logger::Level::DEBUG) << "Finished parsing Huffman table segment [OK]" << std::endl;
        
//          printCurrPos();
    }
    
    void JPEGDecoder::parseSOSSegment()
    {
//         if ( m_huffTableCount < 4 )
//         {
//             //m_huffmanTable
//             //return;
//             m_huffmanTable[1][0] = m_huffmanTable[0][1];
//             m_huffmanTable[1][1] = m_huffmanTable[0][1];
//             
//             m_huffmanTree[1][0].constructHuffmanTree( m_huffmanTable[1][0] );
//             m_huffmanTree[1][1].constructHuffmanTree( m_huffmanTable[1][1] );
//         }
        
        if ( !m_imageFile.is_open() || !m_imageFile.good() )
        {
            LOG(Logger::Level::ERROR) << "Unable scan image file: \'" + m_filename + "\'" << std::endl;
            return;
        }
        
        LOG(Logger::Level::DEBUG) << "Parsing SOS segment..." << std::endl;
        
        UInt16 len;
        
        m_imageFile.read( reinterpret_cast<char *>( &len ), 2 );
        len = htons( len );
        
        LOG(Logger::Level::DEBUG) << "SOS segment length: " << len << std::endl;
        
        UInt8 compCount; // Number of components
        UInt16 compInfo; // Component ID and Huffman table used
        
        m_imageFile >> std::noskipws >> compCount;
        
        if ( compCount < 1 || compCount > 4 )
        {
            LOG(Logger::Level::ERROR) << "Invalid component count in image scan: " << (int)compCount << ", terminating decoding process..." << std::endl;
            return;
        }
        
        LOG(Logger::Level::DEBUG) << "Number of components in scan data: " << (int)compCount << std::endl;
        
        for ( auto i = 0; i < compCount; ++i )
        {
            m_imageFile.read( reinterpret_cast<char *>( &compInfo ), 2 );
            compInfo = htons( compInfo );
            
            UInt8 cID = compInfo >> 8; // 1st byte denotes component ID 
            
            // 2nd byte denotes the Huffman table used:
            // Bits 7 to 4: DC Table #(0 to 3)
            // Bits 3 to 0: AC Table #(0 to 3)
            UInt8 DCTableNum = ( compInfo & 0x00f0 ) >> 4;
            UInt8 ACTableNum = ( compInfo & 0x000f );
            
            LOG(Logger::Level::DEBUG) << "Component ID: " << (int)cID << ", DC Table #: " << (int)DCTableNum << ", AC Table #: " << (int)ACTableNum << std::endl;
        }
        
        // Skip the next three bytes
        for ( auto i = 0; i < 3; ++i )
        {
            UInt8 byte;
            m_imageFile >> std::noskipws >> byte;
        }
        
//         printCurrPos();
        
        LOG(Logger::Level::DEBUG) << "Finished parsing SOS segment [OK]" << std::endl;
        
        scanImageData();
    }
    
    void JPEGDecoder::scanImageData()
    {
        if ( !m_imageFile.is_open() || !m_imageFile.good() )
        {
            LOG(Logger::Level::ERROR) << "Unable scan image file: \'" + m_filename + "\'" << std::endl;
            return;
        }
        
        LOG(Logger::Level::DEBUG) << "Scanning image data..." << std::endl;
        
        UInt8 byte;
        
        while ( m_imageFile >> std::noskipws >> byte )
        {
            if ( byte == JFIF_BYTE_FF )
            {
                UInt8 prevByte = byte;
                
                m_imageFile >> std::noskipws >> byte;
                
                if ( byte == JFIF_EOI )
                {
                    LOG(Logger::Level::INFO) << "Found segment, End of Image (FFD9)" << std::endl;
                    return;
                }
                
                std::bitset<8> bits1( prevByte );
                LOG(Logger::Level::DEBUG) << "0x" << std::hex << std::setfill('0') << std::setw(2)
                                          << std::setprecision(8) << (int)prevByte
                                          << ", Bits: " << bits1 << std::endl;
                                          
                //m_scanData.push_back( bits1 );
                m_scanData.append( bits1.to_string() );
            }
            
            std::bitset<8> bits( byte );
            LOG(Logger::Level::DEBUG) << "0x" << std::hex << std::setfill('0') << std::setw(2)
                                      << std::setprecision(8) << (int)byte
                                      << ", Bits: " << bits << std::endl;
            
            //m_scanData.push_back( std::move( bits ) );
            m_scanData.append( bits.to_string() );
        }
        
        LOG(Logger::Level::DEBUG) << "Finished scanning image data [OK]" << std::endl;
    }
    
    void JPEGDecoder::parseComment()
    {
        if ( !m_imageFile.is_open() || !m_imageFile.good() )
        {
            LOG(Logger::Level::ERROR) << "Unable scan image file: \'" + m_filename + "\'" << std::endl;
            return;
        }
        
        LOG(Logger::Level::DEBUG) << "Parsing comment segment..." << std::endl;
        
        UInt16 lenByte = 0;
        UInt8 byte = 0;
        std::string comment;
        
        m_imageFile.read( reinterpret_cast<char *>(&lenByte), 2 );
        lenByte = htons( lenByte );
        std::size_t curPos = m_imageFile.tellg();
        
        LOG(Logger::Level::DEBUG) << "Comment segment length: " << lenByte << std::endl;
        
        for ( auto i = 0; i < lenByte - 2; ++i )
        {
            m_imageFile >> std::noskipws >> byte;
            
            if ( byte == JFIF_BYTE_FF )
            {
                LOG(Logger::Level::ERROR) << "Unexpected start of marker at offest: " << curPos + i << std::endl;
                LOG(Logger::Level::DEBUG) << "Comment segment content: " << comment << std::endl;
                return;
            }
            
            //std::cout << "Count: " << i << ", Byte: " << (unsigned char)byte << ", Position: " << (std::size_t)m_imageFile.tellg() - 1 <<  std::endl;
            comment.push_back( static_cast<char>(byte) );
        }
        
        LOG(Logger::Level::DEBUG) << "Comment segment content: " << comment << std::endl;
        LOG(Logger::Level::DEBUG) << "Finished parsing comment segment [OK]" << std::endl;
        //std::cout << "Current file pos: " << m_imageFile.tellg() << std::endl;
        
        m_image.setComment( comment );
    }
    
    void JPEGDecoder::byteStuffScanData()
    {
        if ( m_scanData.empty() )
        {
            LOG(Logger::Level::ERROR) << " [ FATAL ] Invalid image scan data" << std::endl;
            return;
        }
        
        LOG(Logger::Level::DEBUG) << "Byte stuffing image scan data..." << std::endl;
        
        for ( unsigned i = 0; i <= m_scanData.size() - 8; i += 8 )
        {
            std::string byte = m_scanData.substr( i, 8 );
            
            if ( byte == "11111111" )
            {
                if ( i + 8 < m_scanData.size() - 8 )
                {
                    std::string nextByte = m_scanData.substr( i + 8, 8 );
                    
                    if ( nextByte == "00000000" )
                    {
                        m_scanData.erase( i + 8, 8 );
                    }
                    else continue;
                }
                else
                    continue;
            }
        }
        
        LOG(Logger::Level::DEBUG) << "Finished byte stuffing image scan data [OK]" << std::endl;
    }
    
    void JPEGDecoder::decodeScanData()
    {
        if ( m_scanData.empty() )
        {
            LOG(Logger::Level::ERROR) << " [ FATAL ] Invalid image scan data" << std::endl;
            return;
        }
        
        byteStuffScanData();
        
        LOG(Logger::Level::DEBUG) << "Decoding image scan data..." << std::endl;
        
        const char* component[] = { "Y (Luminance)", "Cb (Chrominance)", "Cr (Chrominance)" };
        const char* type[] = { "DC", "AC" };        
        
        int MCUCount = ( m_image.getWidth() * m_image.getHeight() ) / 64;
        
//         int maxBitLength = m_scanData.size();
        
        m_MCU.clear();
        //m_MCU.resize( MCUCount );
        LOG(Logger::Level::DEBUG) << "MCU count: " << MCUCount << std::endl;
        
        int k = 0; // The index of the next bit to be scanned
        
        // TODO: Fix redundancy in this part
        for ( auto i = 0; i < MCUCount; ++i )
        {
            LOG(Logger::Level::DEBUG) << "Decoding MCU-" << i + 1 << "..." << std::endl;
            
            // The run-length coding after decoding the Huffman data
            std::array<std::vector<int>, 3> RLE;            
            
            // For each component Y, Cb & Cr, decode 1 DC
            // coefficient and then decode 63 AC coefficients.
            //
            // NOTE:
            // Since a signnificant portion of a RLE for a
            // component contains a trail of 0s, AC coefficients
            // are decoded till, either an EOB (End of block) is
            // encountered or 63 AC coefficients have been decoded.
            
            for ( auto compID = 0; compID < 3; ++compID )
            {
                std::string bitsScanned = ""; // Initially no bits are scanned
                
                // Firstly, decode the Y / DC coefficient
                LOG(Logger::Level::DEBUG) << "Decoding MCU-" << i + 1 << ": " << component[compID] << "/" << type[HT_DC] << std::endl;
                
                int HuffTableID = compID == 0 ? 0 : 1;
                
                while ( 1 )
                {       
                    bitsScanned += m_scanData[k];
                    auto value = m_huffmanTree[HT_DC][HuffTableID].contains( bitsScanned );
                    
                    if ( !isStringWhiteSpace( value ) )
                    {
                        if ( value != "EOB" )
                        {   
                            int zeroCount = UInt8( std::stoi( value ) ) >> 4 ;
                            int category = UInt8( std::stoi( value ) ) & 0x0F;
                            //auto e = m_scanData.substr( k + 1, category );
                            int DCCoeff = bitStringtoValue( m_scanData.substr( k + 1, category ) );
                            
                            //LOG(Logger::Level::DEBUG) << "MCU-" << i + 1 << ": " << component[compID] << "/" << type[HT_DC] << ": ( " << zeroCount << ", " << DCCoeff << " )" << std::endl;
                            
                            k += category + 1;
                            bitsScanned = "";
                            
                            RLE[compID].push_back( zeroCount );
                            RLE[compID].push_back( DCCoeff );
                            
                            break;
                        }
                        
                        else
                        {
                            //LOG(Logger::Level::DEBUG) << "MCU-" << i + 1 << ": " << component[compID] << "/" << type[HT_DC] << ": EOB encountered" << std::endl;
                            bitsScanned = "";
                            k++;
                            
                            //RLE.push_back( 0 );
                            //RLE.push_back( 0 );
                            
                            RLE[compID].push_back( 0 );
                            RLE[compID].push_back( 0 );
                            
                            break;
                        }
                    }
                    else
                        k++;
                }
                
                // Then decode the AC coefficients
                LOG(Logger::Level::DEBUG) << "Decoding MCU-" << i + 1 << ": " << component[compID] << "/" << type[HT_AC] << std::endl;
                bitsScanned = "";
                int ACCodesCount = 0;
                                
                while ( 1 )
                {   
                    // If 63 AC codes have been encountered, this block is done, move onto next block;
                    
                    if ( ACCodesCount  == 63 )
                    {
                        break;
                    }
                    
                    // Append the k-th bit to the bits scanned so far
                    bitsScanned += m_scanData[k];
                    auto value = m_huffmanTree[HT_AC][HuffTableID].contains( bitsScanned );
                    
                    if ( !isStringWhiteSpace( value ) )
                    {
                        if ( value != "EOB" )
                        {
                            int zeroCount = UInt8( std::stoi( value ) ) >> 4 ;
                            int category = UInt8( std::stoi( value ) ) & 0x0F;
                            //auto e = m_scanData.substr( k + 1, category );
                            int ACCoeff = bitStringtoValue( m_scanData.substr( k + 1, category ) );
                            
                            //LOG(Logger::Level::DEBUG) << "AC Code#: " << ACCodesCount + 1 << ", MCU-" << i + 1 << ": " << component[compID] << "/" << type[HT_AC] << ": ( " << zeroCount << ", " << ACCoeff << " )" << std::endl;
                            
                            k += category + 1;
                            bitsScanned = "";
                            
                            RLE[compID].push_back( zeroCount );
                            RLE[compID].push_back( ACCoeff );
                            
                            ACCodesCount += zeroCount + 1;
                        }
                        
                        else
                        {
                            //LOG(Logger::Level::DEBUG) << "MCU-" << i + 1 << ": " << component[compID] << "/" << type[HT_AC] << ": EOB encountered" << std::endl;
                            bitsScanned = "";
                            k++;
                            
                            RLE[compID].push_back( 0 );
                            RLE[compID].push_back( 0 );
                            
                            break;
                        }
                    }
                    
                    else
                        k++;
                }
                
                // If both the DC and AC coefficients are EOB, truncate to (0,0)
                if ( RLE[compID].size() == 2 )
                {
                    bool allZeros = true;
                    
                    for ( auto&& rVal : RLE[compID] )
                    {
                        if ( rVal != 0 )
                        {
                            allZeros = false;
                            break;
                        }
                    }
                    
                    // Remove the extra (0,0) pair
                    if ( allZeros )
                    {
                        RLE[compID].pop_back();
                        RLE[compID].pop_back();
                    }
                }
            }
            
//             if ( i == 62 )
//             {
//                 for ( auto&& rle : RLE )
//                 {
//                     std::string r = "";
//                     for ( auto a = 0; a <= rle.size() - 2; a += 2 )
//                     {
//                         std::stringstream ss;
//                         ss << "( " << rle[a] << ", " << rle[a + 1] << ") ";
//                         r += ss.str();
//                     }
//                     
//                     LOG(Logger::Level::DEBUG) << "RLE 6,7: " << r << std::endl;
//                 }
//             }
            
            // Construct the MCU block from the RLE &
            // quantization tables to a 8x8 matrix
            m_MCU.push_back( MCU( RLE, m_QTables ) );
            
            LOG(Logger::Level::DEBUG) << "Finished decoding MCU-" << i + 1 << " [OK]" << std::endl;
        }
        
        // The remaining bits, if any, in the scan data are discarded as
        // they're added byte align the scan data.
        
        LOG(Logger::Level::DEBUG) << "Finished decoding image scan data [OK]" << std::endl;
    }
}
