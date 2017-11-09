/**
 * @file Image.hpp
 * @author Koushtav Chakrabarty (koushtav@fleptic.eu)
 * @date 15th Oct 2017, 07:20 PM IST
 * @brief The Image struct is an abstract representation of a generic image data
 */

#ifndef IMAGE_HPP
#define IMAGE_HPP

#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <memory>

#include "Types.hpp"
#include "MCU.hpp"

namespace kpeg
{    
    ///// Image structure /////
    
    class Image
    {
        public:
            
            Image();
            
            // Used by decoder
            void createImageFromMCUs( const std::vector<MCU>& MCUVector );
            
            PixelPtr getPixelPtr();
            
            FPixelPtr getFlPixelPtr();
            
            const unsigned getWidth() const;
            
            const unsigned getHeight() const;
            
            const bool dumpRawData( const std::string& filename );
            
            const bool readRawData( const std::string& filename );
            
            void setImageFilename( const std::string& filename );
            
            void setJPEGVersion( const std::string& version );
            
            void setComment( const std::string& comment );
            
            void setDimensions( const std::size_t width, const std::size_t height );
            
        private:
            
            std::string  m_filename;
            PixelPtr     m_pixelPtr;
            FPixelPtr    m_flPixelPtr;
            std::string  m_JPEGversion;
            std::string  m_comment;
            std::size_t  m_width;
            std::size_t  m_height;
    };
    
    const std::string valueToBitString( const Int16 value );
    
    const Int16 bitStringtoValue( const std::string& bitStr );
}

#endif // IMAGE_HPP
