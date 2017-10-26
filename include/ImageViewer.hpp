#ifndef IMAGE_VIEWER_HPP
#define IMAGE_VIEWER_HPP

#include <SFML/Graphics.hpp>

#include "Types.hpp"

namespace kpeg
{
    class ImageViewer
    {
        public:
            
            ImageViewer();
            
            void setImagePtr( const PixelPtr pixptr );
            
            void draw();
            
        private:
            
            sf::Image m_image;
            sf::RenderWindow m_window;
    };
}

#endif // IMAGE_VIEWER_HPP
