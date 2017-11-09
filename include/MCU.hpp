#ifndef MCU_HPP
#define MCU_HPP

#include <array>
#include <vector>
#include <utility>

#include "Types.hpp"
#include "Transform.hpp"

namespace kpeg
{
//     /**
//      * @brief Convert a zig-zag order index to its corresponding matrix indices.
//      */
//     const std::pair<const int, const int> zzOrderToMatIndices( const int zzindex );
//     
//     
//     /**
//      * @brief Convert matrix indices to its corresponding zig-zag order index.
//      */
//     
//     const int matIndicesToZZOrder( const int row, const int column );
    
    
    typedef std::array< std::array< std::array< int, 8 >, 8 > , 3 > CompMatrices;
    
    typedef std::array< std::array< int, 8 >, 8 > Matrix8x8;
    
    
    /**
     * @brief The class MCU handles blocks of 8x8 pixels (aka MCUs) of the image at a time.
     * 
     * This abstracts away the conversion of the decoded RLE-Huffman encoded pixel values
     * to corresponding matrices of 8x8 size. It actually contains three 8x8 matrices to
     * represent the lumninance (Y) and chrominance (Cb & Cr) components.
     * 
     * The MCU object expects as input the RLE-Huffman encoded vector (obtained after
     * the Huffman decoding is done).
     */
    
    class MCU
    {
        public:
            
            MCU();
            
            MCU( const std::array<std::vector<int>, 3>& compRLE,
                 const std::vector<std::vector<UInt16>>& QTables );
            
            void constructMCU( const std::array<std::vector<int>, 3>& compRLE,
                               const std::vector<std::vector<UInt16>>& QTables );
            
            const CompMatrices& getAllMatrices() const;
            
            const Matrix8x8 getYMatrix() const;
            
            const Matrix8x8 getCbMatrix() const;
            
            const Matrix8x8 getCrMatrix() const;
        
        private:
            
            /**
             * @brief Inverse discrete cosine transform
             * 
             * The 8x8 matrices for each component has to be converted
             * back from frequency to spaital domain.
             */
            void computeIDCT();
            
            void performLevelShift();
            
            void convertYCbCrToRGB();
            
        private:
            
            CompMatrices m_8x8block;
            
            static int m_MCUCount;
            
            static std::vector<std::vector<UInt16>> m_QTables;
            
            static int DCDiff[3];
            
            // For storing the IDCT coefficients before level shifting
            std::array< std::array< std::array< float, 8 >, 8 > , 3 > icoeffs;
    };
}

#endif // MCU_HPP
