#include <string>
#include <sstream>
#include <cmath>
#include <iomanip>

#include "MCU.hpp"
#include "Logger.hpp"

namespace kpeg
{
//     const std::pair<const int, const int> zzOrderToMatIndices( const int zzindex )
//     {
//         static const std::pair<const int, const int> zzorder[64] =
//         {
//             {0,0},
//             {0,1}, {1,0},         
//             {2,0}, {1,1}, {0,2},
//             {0,3}, {1,2}, {2,1}, {3,0},
//             {4,0}, {3,1}, {2,2}, {1,3}, {0,4},
//             {0,5}, {1,4}, {2,3}, {3,2}, {4,1}, {5,0},
//             {6,0}, {5,1}, {4,2}, {3,3}, {2,4}, {1,5}, {0,6},
//             {0,7}, {1,6}, {2,5}, {3,4}, {4,3}, {5,2}, {6,1}, {7,0},
//             {7,1}, {6,2}, {5,3}, {4,4}, {3,5}, {2,6}, {1,7},
//             {2,7}, {3,6}, {4,5}, {5,4}, {6,3}, {7,2},
//             {7,3}, {6,4}, {5,5}, {4,6}, {3,7},
//             {4,7}, {5,6}, {6,5}, {7,4},
//             {7,5}, {6,6}, {5,7},
//             {6,7}, {7,6},
//             {7,7}
//         };
//         
//         return zzorder[zzindex];
//     }
//     
//     const int matIndicesToZZOrder( const int row, const int column )
//     {
//         static const int matOrder[8][8] = 
//         {
//             {  0,  1,  5,  6, 14, 16, 27, 28 },
//             {  2,  4,  7, 13, 16, 26, 29, 42 },
//             {  3,  8, 12, 17, 25, 30, 41, 43 },
//             {  9, 11, 18, 24, 31, 40, 44, 53 },
//             { 10, 19, 23, 32, 39, 45, 52, 54 },
//             { 20, 22, 33, 38, 46, 51, 55, 60 },
//             { 21, 34, 37, 47, 50, 56, 59, 61 },
//             { 35, 36, 48, 49, 57, 58, 62, 63 }
//         };
//         
//         return matOrder[row][column];
//     }
    int MCU::m_MCUCount = 0;
    std::vector<std::vector<UInt16>> MCU::m_QTables = {};
    int MCU::DCDiff[3] = { 0, 0, 0 };
    
    MCU::MCU()
    {   
    }
            
    MCU::MCU( const std::array<std::vector<int>, 3>& compRLE, const std::vector<std::vector<UInt16>>& QTables )
    {
        constructMCU( compRLE, QTables );
    }
    
    void MCU::constructMCU( const std::array<std::vector<int>, 3>& compRLE, const std::vector<std::vector<UInt16>>& QTables )
    {
        m_QTables = QTables;
        
        m_MCUCount++;
        
        LOG(Logger::Level::DEBUG) << "Constructing MCU: " << m_MCUCount << "..." << std::endl;
        
//         for ( auto&& rle : compRLE )
//         {
//             std::string rleStr = "";
//             for ( auto v = 0; v < rle.size(); v += 2 )
//             {
//                 std::stringstream ss;
//                 ss << "(" << rle[v] << ", " << rle[v + 1] << ") ";
//                 rleStr += ss.str();
//             }
//             
//             LOG(Logger::Level::DEBUG) << "RLE decoded data: " << rleStr << std::endl;
//         }
        
        const char* component[] = { "Y (Luminance)", "Cb (Chrominance)", "Cr (Chrominance)" };
        const char* type[] = { "DC", "AC" };    
        
        for ( int compID = 0; compID < 3; compID++ )
        {
            //LOG(Logger::Level::DEBUG) << "Constructing matrix for: MCU-" << m_MCUCount << ": " << component[compID] << "..." << std::endl;
            
            // Initialize with all zeros
            std::array<int, 64> zzOrder;            
            std::fill( zzOrder.begin(), zzOrder.end(), 0 );
            int j = -1;
            
            for ( auto i = 0; i <= compRLE[compID].size() - 2; i += 2 )
            {
                if ( compRLE[compID][i] == 0 && compRLE[compID][i + 1] == 0 )
                    break;
                
                j += compRLE[compID][i] + 1; // Skip the number of positions containing zeros
                zzOrder[j] = compRLE[compID][i + 1];
            }
            
            // DC_i = DC_i-1 + DC-difference
            DCDiff[compID] += zzOrder[0];
            zzOrder[0] = DCDiff[compID];
            
            int QIndex = compID == 0 ? 0 : 1;
            for ( auto i = 0; i < 64; ++i )
                zzOrder[i] *= m_QTables[QIndex][i];
            
            // Zig-zag order to 2D matrix order
            for ( auto i = 0; i < 64; ++i )
            {
                auto coords = zzOrderToMatIndices( i );
                
                m_8x8block[compID][ coords.first ][ coords.second ] = zzOrder[i];
            }
            
//             for ( auto&& row : m_8x8block[compID] )
//             {
//                 for ( auto&& val : row )
//                     std::cout << val << "\t";
//                 std::cout << std::endl;
//             }
//             std::cout << std::endl;

//             std::string matrix = "";
//             for ( auto&& row : m_8x8block[compID] )
//             {
//                 for ( auto&& val : row )
//                 {
//                     std::stringstream ss;
//                     ss << std::setw(7) << std::setfill(' ') << val << "";
//                     matrix += ss.str();
//                 }
//                 matrix += "\n";
//             }
//             
//             LOG(Logger::Level::DEBUG) << "DCT Matrix: " << component[compID] << ":-\n" << matrix << std::endl;
        }
        
        computeIDCT();
        performLevelShift();
        convertYCbCrToRGB();
        
        LOG(Logger::Level::DEBUG) << "Finished constructing MCU: " << m_MCUCount << "..." << std::endl;
    }
    
    const CompMatrices& MCU::getAllMatrices() const
    {
        return m_8x8block;
    }
    
    const Matrix8x8 MCU::getYMatrix() const
    {
        return m_8x8block[0];
    }
    
    const Matrix8x8 MCU::getCbMatrix() const
    {
        return m_8x8block[1];
    }
    
    const Matrix8x8 MCU::getCrMatrix() const
    {
        return m_8x8block[2];
    }
    
    void MCU::computeIDCT()
    {
        LOG(Logger::Level::DEBUG) << "Performing IDCT on MCU: " << m_MCUCount << "..." << std::endl;
        
        for ( int i = 0; i <3; ++i )
        {
            for ( int y = 0; y < 8; ++y )
            {
                for ( int x = 0; x < 8; ++x )
                {
                    float sum = 0.0;
                    
                    for ( int u = 0; u < 8; ++u )
                    //for ( int v = 0; v < 8; ++v )
                    {
                        for ( int v = 0; v < 8; ++v )
                        //for ( int u = 0; u < 8; ++u )
                        {
                            float Cu = u == 0 ? 1.0 / std::sqrt(2.0) : 1.0;
                            float Cv = v == 0 ? 1.0 / std::sqrt(2.0) : 1.0;
                            
                            sum += Cu * Cv * m_8x8block[i][u][v] * std::cos( ( 2 * x + 1 ) * u * M_PI / 16.0 ) *
                                            std::cos( ( 2 * y + 1 ) * v * M_PI / 16.0 );
                        }
                    }
                    
                    icoeffs[i][x][y] = 0.25 * sum;
                }
            }
        }
        
//         for (auto&& mat: icoeffs )
//         {
//             for ( auto&& row : mat )
//             {
//                 for ( auto&& v : row )
//                     //std::cout << std::roundl( v ) + 128 << "\t";
//                     std::cout << std::roundl( v ) << "\t";
//                 std::cout << std::endl;
//             }
//             std::cout << std::endl;
//         }

        LOG(Logger::Level::DEBUG) << "IDCT of MCU: " << m_MCUCount << " complete [OK]" << std::endl;
    }
    
    void MCU::performLevelShift()
    {
        LOG(Logger::Level::DEBUG) << "Performing level shift on MCU: " << m_MCUCount << "..." << std::endl;
        
        for ( int i = 0; i <3; ++i )
        {
            for ( int y = 0; y < 8; ++y )
            {
                for ( int x = 0; x < 8; ++x )
                {
                    m_8x8block[i][y][x] = std::roundl( icoeffs[i][y][x] ) + 128;
                }
            }
        }
        
//         for (auto&& mat: m_8x8block )
//         {
//             for ( auto&& row : mat )
//             {
//                 for ( auto&& v : row )
//                     std::cout << v << "\t";
//                 std::cout << std::endl;
//             }
//             std::cout << std::endl;
//         }
        
        LOG(Logger::Level::DEBUG) << "Level shift on MCU: " << m_MCUCount << " complete [OK]" << std::endl;
    }
    
    void MCU::convertYCbCrToRGB()
    {
        LOG(Logger::Level::DEBUG) << "Converting from Y-Cb-Cr colorspace to R-G-B colorspace for MCU: " << m_MCUCount << "..." << std::endl;
        
        for ( int y = 0; y < 8; ++y )
        {
            for ( int x = 0; x < 8; ++x )
            {
                float Y = m_8x8block[0][y][x];
                float Cb = m_8x8block[1][y][x];
                float Cr = m_8x8block[2][y][x];
                
                int R = (int)std::floor( Y + 1.402 * ( 1.0 * Cr - 128.0 ) );
                int G = (int)std::floor( Y - 0.344136 * ( 1.0 * Cb - 128.0 ) - 0.714136 * ( 1.0 * Cr - 128.0 ) );
                int B = (int)std::floor( Y + 1.772 * ( 1.0 * Cb - 128.0 ) );
                
                R = std::max( 0, std::min( R, 255 ) );
                G = std::max( 0, std::min( G, 255 ) );
                B = std::max( 0, std::min( B, 255 ) );
                
                m_8x8block[0][y][x] = R;
                m_8x8block[1][y][x] = G;
                m_8x8block[2][y][x] = B;
                
//                 std::cout << "(" << m_8x8block[0][y][x]
//                           << "," << m_8x8block[1][y][x] << ","
//                           << m_8x8block[2][y][x] << ") ";
            }
//             std::cout << std::endl;
        }
        
        LOG(Logger::Level::DEBUG) << "Colorspace conversion for MCU: " << m_MCUCount << " done [OK]" << std::endl;
    }
}
