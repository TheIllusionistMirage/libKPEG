/// Transform helper module

#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP

#include <string>
#include <utility>

#include "Types.hpp"

namespace kpeg
{
    /// Convert a zig-zag order index to its corresponding matrix indices
    ///
    /// @param zzIndex the index in the zig-zag order
    /// @return the matrix indices corresponding to the zig-zag order
    const std::pair<const int, const int> zzOrderToMatIndices(const int zzindex);
    
    /// Convert matrix indices to its corresponding zig-zag order index
    ///
    /// @param the matrix indices
    /// @return the zig-zag index corresponding to the matrix indices
    const int matIndicesToZZOrder(const int row, const int column);

    /// Convert a value to its appropriate bit string
    ///
    /// @param value the value to convert
    /// @return the bis string representation
    const std::string valueToBitString(const kpeg::types::Int16 value);
    
    /// Convert a bit strig to it's corresponding value
    ///
    /// @param bitStr the bit string
    /// @return the value corresponding to the bit string
    const kpeg::types::Int16 bitStringtoValue(const std::string& bitStr);
    
    /// Get the category of a value
    ///
    /// @param value the whose category has to be determined
    /// @return the category of the specified value
    const kpeg::types::Int16 getValueCategory(const kpeg::types::Int16 value);
}

#endif // TRANSFORM_HPP
