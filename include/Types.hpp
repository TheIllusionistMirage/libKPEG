#ifndef TYPES_HPP
#define TYPES_HPP

#include <vector>
#include <array>
#include <utility>
#include <memory>

namespace kpeg
{
    ///// Pixel components  /////
    
    typedef unsigned char  UInt8;
    typedef unsigned short UInt16;
    typedef unsigned int   UInt32;
    
    typedef char  Int8;
    typedef short Int16;
    typedef int   Int32;
    
    enum Components
    {
        COMP1 ,
        COMP2 ,
        COMP3
    };
    
    enum RGBComponents
    {
        RED   ,
        GREEN ,
        BLUE
    };
    
    enum YCbCrComponents
    {
        Y  ,
        Cb ,
        Cr
    };
    
    
    ///// Pixel structures /////
    
    /**
     * @brief Pixel with integer component values
     * 
     * A Pixel type denotes an abstract pixel with integer valued components.
     * In general, a Pixel contains three components for denoting the intensities
     * of the respective axes in its color space.
     */
    struct Pixel
    {
        /**
         * Default constructor
         */
        Pixel()
        {
            comp[0] = comp[1] = comp[2]  = 0;
        }
        
        /*
         * @brief Parameterized constructor
         * @param comp1 - Intensity value of pixel component 1
         * @param comp2 - Intensity value of pixel component 2
         * @param comp3 - Intensity value of pixel component 3
         */
        Pixel( const Int16 comp1, const Int16 comp2, const Int16 comp3 )
        {
            comp[0] = comp1;
            comp[1] = comp2;
            comp[2] = comp3;
        }
        
        Int16 comp[3]; // Stores the intensity components of the pixel
    };
    
    /**
     * @brief Pixel with floating point component values
     * 
     * A FPixel type denotes an abstract pixel with float valued components.
     * In general, a Pixel contains three components for denoting the intensities
     * of the respective axes in its color space.
     */
    struct FPixel
    {
        /*
         * Default constructor
         */
        FPixel()
        {
            comp[0] = comp[1] = comp[2]  = 0.f;
        }
        
        /*
         * @brief Parameterized constructor
         * @param comp1 - Intensity value of pixel component 1
         * @param comp2 - Intensity value of pixel component 2
         * @param comp3 - Intensity value of pixel component 3
         */        
        FPixel( const float comp1, const float comp2, const float comp3 )
        {
            comp[0] = comp1;
            comp[1] = comp2;
            comp[2] = comp3;
        }
        
        float comp[3]; // Stores the intensity components of the pixel
    };
    
    /** 2D Pixel array types */
    typedef std::shared_ptr<std::vector<std::vector<Pixel>>>  PixelPtr;
    typedef std::shared_ptr<std::vector<std::vector<FPixel>>> FPixelPtr;
    
    /** Huffman table */
    typedef std::array<std::pair<int, std::vector<UInt8>>, 16> HuffmanTable;
    
    /** 
     * Constants used to access a Huffman table based on the class and ID
     * E.g., To access the Huffman table for the DC coefficients of the
     * CbCr component, we use `huff_table[HT_DC][HT_CbCr]`.
     */
    const int HT_DC   = 0;
    const int HT_AC   = 1;
    const int HT_Y    = 0;
    const int HT_CbCr = 1;
}

#endif // TYPES_HPP
