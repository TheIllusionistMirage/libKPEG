#ifndef MCU_HPP
#define MCU_HPP

#include <array>
#include <vector>
#include <utility>

#include "Types.hpp"

namespace kpeg
{
    /**
     * @brief Convert a zig-zag order index to its corresponding matrix indices.
     */
    static const std::pair<const int, const int> zzOrderToMatIndices( const int zzindex )
    {
        static const std::pair<const int, const int> zzorder[64] =
        {
            {0,0},
            {0,1}, {1,0},         
            {2,0}, {1,1}, {0,2},
            {0,3}, {1,2}, {2,1}, {3,0},
            {4,0}, {3,1}, {2,2}, {1,3}, {0,4},
            {0,5}, {1,4}, {2,3}, {3,2}, {4,1}, {5,0},
            {6,0}, {5,1}, {4,2}, {3,3}, {2,4}, {1,5}, {0,6},
            {0,7}, {1,6}, {2,5}, {3,4}, {4,3}, {5,2}, {6,1}, {7,0},
            {7,1}, {6,2}, {5,3}, {4,4}, {3,5}, {2,6}, {1,7},
            {2,7}, {3,6}, {4,5}, {5,4}, {6,3}, {7,2},
            {7,3}, {6,4}, {5,5}, {4,6}, {3,7},
            {4,7}, {5,6}, {6,5}, {7,4},
            {7,5}, {6,6}, {5,7},
            {6,7}, {7,6},
            {7,7}
        };
        
        return zzorder[zzindex];
    }
    
    
    /**
     * @brief Convert matrix indices to its corresponding zig-zag order index.
     */
    
    static const int matIndicesToZZOrder( const int row, const int column )
    {
        static const int matOrder[8][8] = 
        {
            {  0,  1,  5,  6, 14, 16, 27, 28 },
            {  2,  4,  7, 13, 16, 26, 29, 42 },
            {  3,  8, 12, 17, 25, 30, 41, 43 },
            {  9, 11, 18, 24, 31, 40, 44, 53 },
            { 10, 19, 23, 32, 39, 45, 52, 54 },
            { 20, 22, 33, 38, 46, 51, 55, 60 },
            { 21, 34, 37, 47, 50, 56, 59, 61 },
            { 35, 36, 48, 49, 57, 58, 62, 63 }
        };
        
        return matOrder[row][column];
    }
    
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
