#include "KPEG.hpp"
#include "Decoder.hpp"
#include "Encoder.hpp"
#include "Logger.hpp"

namespace kpeg
{
    void KPEG::decodeFile(const std::string& filename)
    {
        LOG(Logger::Level::INFO) << "JPEG Decoder invoked." << std::endl;
        
        JPEGDecoder decoder;
        decoder.decodeImageFile();
        decoder.displayImage();
        
        LOG(Logger::Level::INFO) << "JPEG Decoder finished." << std::endl;
    }
    
    void KPEG::encodeFile( const std::string& filenameIn,
                           const std::string& filenameOut )
    {
        
    }
}
