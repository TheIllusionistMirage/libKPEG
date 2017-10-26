#ifndef KPEG_HPP
#define KPEG_HPP

#include <fstream>

namespace kpeg
{
    class KPEG
    {
        public:
            
            static void decodeFile( const std::string& filename );
            
            static void encodeFile( const std::string& filenameIn,
                                    const std::string& filenameOut );
            
        private:                        
    };
}


#endif // KPEG_HPP
