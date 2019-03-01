/// Image module
///
/// Abstraction representing a raw uncompressed image

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
    /// Image is an abstraction for a raw, uncompressed image
    ///
    /// A raw, uncompressed image is nothing but a 2D array of pixels
    class Image
    {
        public:
            
            /// Default constructor
            Image();
            
            /// Create an image from a list of MCUs
            ///
            /// @param MCUs list of minimum coded units that can be converted to an image
            void createImageFromMCUs(const std::vector<MCU>& MCUs);
            
            /// Get the 2D array of pixels that comprise the image
            ///
            /// The individual pixels in have a discrete range
            ///
            /// @return a 2D array of pixels that represents the image
            kpeg::types::PixelPtr getPixelPtr();
            
            /// Get the 2D array of pixels that comprise the image
            ///
            /// The individual pixels in have a continuous range
            ///
            /// @return a 2D array of pixels that represents the image
            kpeg::types::FPixelPtr getFlPixelPtr();
            
            /// Get the width of the image in pixels
            ///
            /// @return the width of the image
            const unsigned getWidth() const;
            
            /// Get the height of the image in pixels
            ///
            /// @return the height of the image
            const unsigned getHeight() const;
            
            /// Write the raw, uncompressed image data to specified file on the disk.
            ///
            /// The data written is in PPM format
            ///
            /// @param filename the location in the disk to write the image data
            /// @return true if succeeds in writing, else false
            const bool dumpRawData(const std::string& filename);
            
            /// Read the raw image data to specified file on the disk.
            ///
            /// The data read is from a PPM file
            ///
            /// @param filename the location in the disk to read the image data from
            /// @return true if succeeds in reading, else false
            const bool readRawData(const std::string& filename);
            
            /// Sets the file name of the image file
            ///
            /// @param filename the name of the image file
            void setImageFilename(const std::string& filename);
            
            /// Sets the version of JPEG compression algorithm used
            ///
            /// @param version the version to use
            void setJPEGVersion(const std::string& version);
            
            /// Sets the comment section in the JFIF file
            ///
            /// @param comment the comment to add
            void setComment(const std::string& comment);

            /// Sets the dimensions of the image
            void setDimensions(const std::size_t width, const std::size_t height);
            
        private:
            
            /// File name of the image on disk
            std::string m_filename;
            
            /// The 2D pixel array with discrete range
            kpeg::types::PixelPtr m_pixelPtr;

            /// The 2D pixel array with continuous range
            kpeg::types::FPixelPtr m_flPixelPtr;

            /// The version of JPEG compression algorithm being used
            std::string m_JPEGversion;

            /// The comment added to the JFIF file
            std::string m_comment;

            /// Width of the image
            std::size_t m_width;

            /// Height of the image
            std::size_t m_height;
    };
}

#endif // IMAGE_HPP
