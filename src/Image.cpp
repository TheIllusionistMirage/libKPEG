/// Image module implementation

#include <arpa/inet.h> // htons
#include <string>
#include <cmath>

#include "Image.hpp"
#include "Logger.hpp"

namespace kpeg
{
    Image::Image() :
        m_filename{""} ,
        m_pixelPtr{nullptr} ,
        m_flPixelPtr{nullptr} ,
        m_JPEGversion{""} ,
        m_comment{""}
    {
        LOG(Logger::Level::INFO) << "Created new Image object" << std::endl;
    }
    
    void Image::createImageFromMCUs(const std::vector<MCU>& MCUs)
    {
        LOG(Logger::Level::INFO) << "Creating Image from MCU vector..." << std::endl;
        
        int mcuNum = 0;
        
        int jpegWidth = m_width % 8 == 0 ? m_width : m_width + 8 - (m_width % 8);
        int jpegHeight = m_height % 8 == 0 ? m_height : m_height + 8 - (m_height % 8);
        
        // Create a pixel pointer of size (Image width) x (Image height)
        m_pixelPtr = std::make_shared<std::vector<std::vector<kpeg::types::Pixel>>>(
            jpegHeight, std::vector<kpeg::types::Pixel>(jpegWidth, kpeg::types::Pixel()));
        
        // Populate the pixel pointer based on data from the specified MCUs
        for (int y = 0; y <= jpegHeight - 8; y += 8)
        {
            for (int x = 0; x <= jpegWidth - 8; x += 8)
            {
                auto pixelBlock = MCUs[mcuNum].getAllMatrices();
                
                for (int v = 0; v < 8; ++v)
                {
                    for (int u = 0; u < 8; ++u)
                    {
                        (*m_pixelPtr)[y + v][x + u].comp[0] = pixelBlock[0][v][u]; // R
                        (*m_pixelPtr)[y + v][x + u].comp[1] = pixelBlock[1][v][u]; // G
                        (*m_pixelPtr)[y + v][x + u].comp[2] = pixelBlock[2][v][u]; // B
                    }
                }
            
                mcuNum++;
            }
        }
        
        // Trim the image width to nearest multiple of 8
        if (m_width != jpegWidth)
        {
            for (auto&& row : *m_pixelPtr)
                for (int c = 0; c < 8 - m_width % 8; ++c)
                    row.pop_back();
        }
        
        // Trim the image height to nearest multiple of 8
        if (m_height != jpegHeight)
        {
            for (int c = 0; c < 8 - m_height % 8; ++c)
                m_pixelPtr->pop_back();
        }        

        LOG(Logger::Level::INFO) << "Finished created Image from MCU [OK]" << std::endl;
    }
    
    kpeg::types::PixelPtr Image::getPixelPtr()
    {
        return m_pixelPtr;
    }
    
    kpeg::types::FPixelPtr Image::getFlPixelPtr()
    {
        return m_flPixelPtr;
    }

    const unsigned int Image::getWidth() const
    {
        return m_width;
    }

    const unsigned int Image::getHeight() const
    {
        return m_height;
    }

    const bool Image::dumpRawData(const std::string& filename)
    {
        if (m_pixelPtr == nullptr)
        {
            LOG(Logger::Level::ERROR) << "Unable to create dump file \'" + filename + "\', Invalid pixel pointer" << std::endl;
            return false;
        }
        
        std::ofstream dumpFile(filename, std::ios::out);
        
        if (!dumpFile.is_open() || !dumpFile.good())
        {
            LOG(Logger::Level::ERROR) << "Unable to create dump file \'" + filename + "\'." << std::endl;
            return false;
        }
        
        dumpFile << "P6" << std::endl;
        dumpFile << "# PPM dump created using libKPEG: https://github.com/TheIllusionistMirage/libKPEG" << std::endl;
        dumpFile << m_width << " " << m_height << std::endl;
        dumpFile << 255 << std::endl;
        
        for (auto&& row : *m_pixelPtr)
        {
            for (auto&& pixel : row)
                dumpFile << (kpeg::types::UInt8)pixel.comp[kpeg::types::RGBComponents::RED]
                         << (kpeg::types::UInt8)pixel.comp[kpeg::types::RGBComponents::GREEN]
                         << (kpeg::types::UInt8)pixel.comp[kpeg::types::RGBComponents::BLUE];
        }
        
        LOG(Logger::Level::INFO) << "Raw image data dumped to file: \'" + filename + "\'." << std::endl;
        dumpFile.close();
        return true;
    }
    
    const bool Image::readRawData(const std::string& filename)
    {
        std::ifstream rawImgFile(filename, std::ios::in | std::ios::binary);
        
        if (!rawImgFile.is_open() || !rawImgFile.good())
        {
            LOG(Logger::Level::ERROR) << "Unable to read PPM file: \'" + filename + "\'" << std::endl;
            return false;
        }
        
        kpeg::types::UInt16 magicBytes;
        kpeg::types::UInt8 byte;
        
        rawImgFile.read(reinterpret_cast<char *>(&magicBytes), 2);
        magicBytes = htons(magicBytes);
        
        if (magicBytes != 0x5036)
        {
            LOG(Logger::Level::ERROR) << "Invalid PPM file: \'" + filename + "\'" << std::endl;
            return false;
        }
        
        // Check for comments if any
        rawImgFile >> byte;
        
        // If '#' encountered, rest of the values till a LF are ignored
        if (byte == 0x23)
        {
            do 
            {
                rawImgFile >> std::noskipws >> byte;
            }
            while (byte != 0x0A); // CR encountered
        }
        
        // Extract the image dimensions (width x height)
        unsigned int width;
        unsigned int height;
        
        rawImgFile >> std::skipws >> width;
        rawImgFile >> std::skipws >> height;
        
        m_width = width;
        m_height = height;
        
        LOG(Logger::Level::INFO) << "Width: " << width << std::endl;
        LOG(Logger::Level::INFO) << "Height: " << height << std::endl;
        
        m_flPixelPtr = std::make_shared<std::vector<std::vector<kpeg::types::FPixel>>>(height, std::vector<kpeg::types::FPixel>(width, kpeg::types::FPixel()));
        int row = 0, col = 0;
        
        // Extract the maximum intensity level
        unsigned int maxIntensity;
        rawImgFile >> std::skipws >> maxIntensity;
        LOG(Logger::Level::INFO) << "Maximum intensity: " << maxIntensity << std::endl;
        
        int curPos = rawImgFile.tellg();
        
        while ((int)rawImgFile.tellg() - curPos < width * height * 3 * 3)
        {
            kpeg::types::UInt8 R, G, B;
            
            rawImgFile >> std::skipws >> R;
            rawImgFile >> std::skipws >> G;
            rawImgFile >> std::skipws >> B;
            
            (*m_flPixelPtr)[row][col++] = kpeg::types::FPixel(R, G, B);
            
            if (col == width)
            {
                row++;
                col = 0;
            }
            
            if (row == height)
            {
                break;
            }
        }
        
        LOG(Logger::Level::INFO) << "Reading raw image data complete" << std::endl;
        rawImgFile.close();        
        m_filename = filename;
        
        return true;
    }
    
    void Image::setImageFilename(const std::string& filename)
    {
        m_filename = filename;
    }

    void Image::setJPEGVersion(const std::string& version)
    {
        m_JPEGversion = version;
    }

    void Image::setComment(const std::string& comment)
    {
        m_comment = comment;
    }
    
    void Image::setDimensions(const std::size_t width, const std::size_t height)
    {
        m_width = width;
        m_height = height;
    }
}
