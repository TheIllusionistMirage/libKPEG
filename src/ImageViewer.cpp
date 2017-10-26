#include "ImageViewer.hpp"
#include "Logger.hpp"

namespace kpeg
{
    ImageViewer::ImageViewer() :
     m_image{} ,
     m_window{}
    {
        LOG(Logger::Level::INFO) << "Created ImageViewer object" << std::endl;
    }
    
    void ImageViewer::setImagePtr( const PixelPtr pixptr )
    {
        LOG(Logger::Level::INFO) << "Initializing ImageViewer with pixel data..." << std::endl;
        
        int width = pixptr->back().size();
        int height = pixptr->size();
        
        // Create a blank (Image width) x (Image height) sf::Image
        // object with all pixels set to RGB(0, 0, 0)
        
        m_image.create( width, height, sf::Color::Black );
        
        // Assign the sf::Image object pixel data from the decoded
        // kpeg::Image's pixel pointer
        
        for ( int y = 0; y < height; ++y )
        {
            for ( int x = 0; x < width; ++x )
            {
                sf::Color color{ static_cast<UInt8>( (*pixptr)[y][x].comp[0] ) ,
                                 static_cast<UInt8>( (*pixptr)[y][x].comp[1] ) ,
                                 static_cast<UInt8>( (*pixptr)[y][x].comp[2] ) };
                m_image.setPixel( x, y, sf::Color( color ) );
            }
        }
        
        LOG(Logger::Level::INFO) << "ImageViewer initialized with pixel data [OK]" << std::endl;
    }
    
    void kpeg::ImageViewer::draw(  )
    {
        m_window.create( sf::VideoMode{ m_image.getSize().x, m_image.getSize().y }, "JPEG(Baseline, DCT) ImageViewer v0.01" );
        
        sf::Texture imageTex;
        imageTex.loadFromImage( m_image );
        
        sf::Sprite imageSprite( imageTex );
        
        while ( m_window.isOpen() )
        {
            sf::Event event;
            
            while ( m_window.pollEvent(event) )
            {
                if ( event.type == sf::Event::Closed )
                    m_window.close();
                
                else if ( event.type == sf::Event::KeyPressed )
                {
                    if ( event.key.code == sf::Keyboard::Escape )
                        m_window.close();
                }
            }
            
            m_window.clear( sf::Color::Black );
            m_window.draw( imageSprite );
            m_window.display();
        }
    }
}
