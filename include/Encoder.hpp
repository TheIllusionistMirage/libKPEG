/**
 * @file Encoder.hpp
 * @author Koushtav Chakrabarty (koushtav@fleptic.eu)
 * @date 1000 B.C.
 * @brief The implementation of a baseline DCT JPEG encoder
 * 
 * Encoder module is the implementation of a 8-bit Sequential
 * Baseline DCT, grayscale/RGB encoder with no subsampling (4:4:4).
 */

#ifndef ENCODER_HPP
#define ENCODER_HPP

#include <fstream>
#include <vector>
#include <utility>
#include <bitset>

#include "Types.hpp"
#include "Transform.hpp"
#include "Image.hpp"
#include "HuffmanTree.hpp"
#include "MCU.hpp"

namespace kpeg
{
    // Luminance Quantization matrix, Quality: 50%
    const std::array<std::array<uint8_t, 8>, 8> M_QT_MAT_LUMA
    {
        {
            { 16, 11, 10, 16,  24,  40,  51, 61  },
            { 12, 12, 14, 19,  26,  58,  60, 55  },
            { 14, 13, 16, 24,  40,  57,  69, 56  },
            { 14, 17, 22, 29,  51,  87,  80, 62  },
            { 18, 22, 37, 56,  86, 109, 103, 77  },
            { 24, 35, 55, 64,  81, 104, 113, 92  },    
            { 49, 64, 78, 87, 103, 121, 120, 101 },
            { 72, 92, 95, 98, 112, 100, 103, 99  }    
        }
    };
    
    // Chrominance Quantization matrix, Quality: 50%
    const std::array<std::array<uint8_t, 8>, 8> M_QT_MAT_CHROMA
    {
        {
            { 17, 18, 24, 47, 99, 99, 99, 99 },
            { 18, 21, 26, 66, 99, 99, 99, 99 },
            { 24, 26, 56, 99, 99, 99, 99, 99 },
            { 47, 66, 99, 99, 99, 99, 99, 99 },
            { 99, 99, 99, 99, 99, 99, 99, 99 },
            { 99, 99, 99, 99, 99, 99, 99, 99 },
            { 99, 99, 99, 99, 99, 99, 99, 99 },
            { 99, 99, 99, 99, 99, 99, 99, 99 },
        }
    };
    
    // Encoder
    class JPEGEncoder
    {
        public:
        
            enum ResultCode
            {
                SUCCESS ,
                TERMINATE ,
                ERROR ,
                ENCODE_INCOMPLETE ,
                ENCODE_DONE
            };
        
        public:
            
            JPEGEncoder();
            
            JPEGEncoder( const std::string& filename );
            
            ~JPEGEncoder();
            
            bool open( const std::string& filename );
            
            bool encodeImage();
            
            bool saveToJFIFFile();
            
        private:
            
            void transformColorspace();
            
            void levelShiftComponents();
            
            void computeDCT();
            
            void quantize();
            
            std::array<std::vector<int>, 3> generateRLE( const int y, const int x );
        
        private:
            
            std::string m_filename;
            
            std::ifstream m_imageFile;
            
            std::ofstream m_outputJPEG;
            
            Image m_image;
            
            std::vector<std::vector<UInt16>> m_QTables;
            
            std::vector<MCU> m_MCU;
    };
}

#endif // ENCODER_HPP
