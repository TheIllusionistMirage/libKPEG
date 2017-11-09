#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP

#include <utility>

namespace kpeg
{
    /**
     * @brief Convert a zig-zag order index to its corresponding matrix indices.
     */
    const std::pair<const int, const int> zzOrderToMatIndices( const int zzindex );
    
    
    /**
     * @brief Convert matrix indices to its corresponding zig-zag order index.
     */
    
    const int matIndicesToZZOrder( const int row, const int column );
}

#endif // TRANSFORM_HPP
