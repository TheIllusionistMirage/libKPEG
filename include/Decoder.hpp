/**
 * @file Decoder.hpp
 * @author Koushtav Chakrabarty (koushtav@fleptic.eu)
 * @date 2000 B.C.
 * @brief The implementation of a baseline, DCT JPEG decoder
 * 
 * Decoder module is the implementation of a 8-bit Sequential
 * Baseline DCT, grayscale/RGB encoder with no subsampling (4:4:4).
 */

#ifndef DECODER_HPP
#define DECODER_HPP

#include <fstream>
#include <vector>
#include <utility>
#include <bitset>

#include "Types.hpp"
#include "Image.hpp"
#include "HuffmanTree.hpp"
#include "MCU.hpp"

namespace kpeg
{
    class JPEGDecoder
    {
        public:
            enum ResultCode
            {
                SUCCESS ,
                TERMINATE ,
                ERROR ,
                DECODE_INCOMPLETE ,
                DECODE_DONE
            };
        
        public:
            
            ResultCode decodeImageFile();
            
//             void displayImage();
            
        public:
            
            JPEGDecoder();
            
            JPEGDecoder( const std::string& filename );
            
            ~JPEGDecoder();
            
            bool open( const std::string& filename );
            
            void close();
            
            ResultCode parseSegmentInfo( const UInt8 byte );
            
            void printDetectedSegmentNames();
            
            bool dumpRawData();
            
            inline void printCurrPos()
            {
                std::cout << "Current file pos: 0x" << std::hex << m_imageFile.tellg() << std::endl;
            }
                        
        private:
            
            void parseJFIFSegment();
            
            void parseQuantizationTable();
            
            ResultCode parseSOF0Segment();
            
            void parseHuffmanTable();
            
            void parseSOSSegment();
            
            void scanImageData();
            
            void parseComment();
            
            //
            
            // Convert bytes of the form XXFF00YY to just XXFFYY
            void byteStuffScanData();
            
            /**
             * @brief Decode the RLE-Huffman encoded image pixel data
             * @author Koushtav Chakrabarty (koushtav@fleptic.eu)
             * 
             * This function goes bit by bit over the image scan data
             * and decodes it using the provided DC and AC Huffman tables
             * for luminance (Y) and chrominance ( Cb & Cr ).
             */
            void decodeScanData();
            
        private:
            
            void displayHuffmanCodes();
            
        private:
            
            std::string m_filename;
            
            std::ifstream m_imageFile;
            
            Image m_image;
            
            std::vector<std::vector<UInt16>> m_QTables;//[4];
            
            //int m_huffTableCount;
            
            // For i=0..3:
            //    HT_i is array of size=16, where j-th element is < count-j-bits, symbol-list >
            //
            HuffmanTable m_huffmanTable[2][2];
            
            std::vector< std::pair<int, int> > mDHTsScanned;
            
            HuffmanTree m_huffmanTree[2][2];
            
            // Image scan data
            //std::vector<std::bitset<8>> m_scanData;
            std::string m_scanData;
            
            std::vector<MCU> m_MCU;
    };
}

#endif // DECODER_HPP
