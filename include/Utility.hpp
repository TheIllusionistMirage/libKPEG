#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <string>
#include <cctype>

namespace kpeg
{
    ///// String helpers  /////
    
    inline const bool isValidChar(const char ch)
    {
        return isprint(ch);
    }
    
    inline const bool isValidFilename(const std::string& filename)
    {
        for ( auto&& c : filename )
            if ( !isValidChar(c) )
                return false;
            
        return true;
    }
    
    inline const bool isWhiteSpace(const char ch)
    {
        return iscntrl(ch) || isblank(ch) || isspace(ch);
    }
    
    inline const bool isStringWhiteSpace( const std::string& str )
    {
        
        for ( auto&& c : str )
            if ( !isWhiteSpace(c) )
                return false;
        return true;
    }
}

#endif // UTILITY_HPP
