/// A simple abstraction for the decoder of a JPEG decoder
///
/// Decoder module is the implementation of a 8-bit Sequential
/// Baseline DCT, grayscale/RGB encoder with no subsampling (4:4:4)

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
    class Decoder
    {
        public:

            /// The result of a decode operation
            enum ResultCode
            {
                SUCCESS,
                TERMINATE,
                ERROR,
                DECODE_INCOMPLETE,
                DECODE_DONE
            };
        
        public:
            
            ResultCode decodeImageFile();
            
        public:
            
            /// Default constructor
            Decoder();
            
            /// Initialize decoder with info about image file
            Decoder(const std::string& filename);
            
            /// Destructor
            ~Decoder();
            
            /// Open a JFIF image file for decoding
            bool open(const std::string& filename);
            
            /// Close the JFIF file
            void close();
            
            /// Parse the info of the specified segment in the JFIF file
            ResultCode parseSegmentInfo(const kpeg::types::UInt8 byte);
            
            // /// 
            // void printDetectedSegmentNames();
            
            /// Write raw, uncompressed image data to disk in PPM format
            bool dumpRawData();
            
            // /// Print the current position (in terms of bytes) in the file
            // inline void printCurrPos()
            // {
            //     std::cout << "Current file pos: 0x" << std::hex << m_imageFile.tellg() << std::endl;
            // }
                        
        private:
            
            /// Parse the JFIF segment at the very beginning of the JFIF file
            void parseJFIFSegment();
            
            /// Parse the quantization tables specified in the JFIF file
            void parseQuantizationTable();
            
            /// Parse the Start of File segment
            ResultCode parseSOF0Segment();
            
            /// Parse the Huffman tables specified in the JFIF file
            void parseHuffmanTable();
            
            /// Parse the start of scan segment in the JFIF file
            void parseSOSSegment();
            
            /// Parse the actual compressed image data stored in the JFIF file
            void scanImageData();
            
            /// Parse the comment in the JFIF file
            void parseComment();
            
            // Convert bytes of the form XXFF00YY to just XXFFYY
            void byteStuffScanData();
            
            /// Decode the RLE-Huffman encoded image pixel data
            ///
            /// This function goes bit by bit over the image scan data
            /// and decodes it using the provided DC and AC Huffman tables
            /// for luminance (Y) and chrominance (Cb & Cr)
            void decodeScanData();
            
        private:
            
            // void displayHuffmanCodes();
            
        private:
            
            std::string m_filename;
            
            std::ifstream m_imageFile;
            
            Image m_image;
            
            std::vector<std::vector<kpeg::types::UInt16>> m_QTables;
            
            // For i=0..3:
            //    HT_i is array of size=16, where j-th element is < count-j-bits, symbol-list >
            //
            kpeg::types::HuffmanTable m_huffmanTable[2][2];
            
            std::vector< std::pair<int, int> > mDHTsScanned;
            
            HuffmanTree m_huffmanTree[2][2];
            
            // Image scan data
            std::string m_scanData;
            
            std::vector<MCU> m_MCU;
    };
}

#endif // DECODER_HPP
