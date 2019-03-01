#include <string>
#include <sstream>
#include <cmath>
#include <iomanip>

#include "MCU.hpp"
#include "Logger.hpp"

namespace kpeg
{
    int MCU::m_MCUCount = 0;
    std::vector<std::vector<kpeg::types::UInt16>> MCU::m_QTables = {};
    int MCU::DCDiff[3] = { 0, 0, 0 };
    
    MCU::MCU()
    {   
    }
            
    MCU::MCU( const std::array<std::vector<int>, 3>& compRLE, const std::vector<std::vector<kpeg::types::UInt16>>& QTables )
    {
        constructMCU( compRLE, QTables );
    }
    
    void MCU::constructMCU( const std::array<std::vector<int>, 3>& compRLE, const std::vector<std::vector<kpeg::types::UInt16>>& QTables )
    {
        m_QTables = QTables;
        
        m_MCUCount++;
        
        LOG(Logger::Level::DEBUG) << "Constructing MCU: " << m_MCUCount << "..." << std::endl;
        
        const char* component[] = { "Y (Luminance)", "Cb (Chrominance)", "Cr (Chrominance)" };
        const char* type[] = { "DC", "AC" };    
        
        for ( int compID = 0; compID < 3; compID++ )
        {
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
            for ( auto i = 0; i < 64; ++i ) // !!!!!! i = 1
                zzOrder[i] *= m_QTables[QIndex][i];
            
            // Zig-zag order to 2D matrix order
            for ( auto i = 0; i < 64; ++i )
            {
                auto coords = zzOrderToMatIndices( i );
                
                m_8x8block[compID][ coords.first ][ coords.second ] = zzOrder[i];
            }
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
                    {
                        for ( int v = 0; v < 8; ++v )
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
            }
        }
        
        LOG(Logger::Level::DEBUG) << "Colorspace conversion for MCU: " << m_MCUCount << " done [OK]" << std::endl;
    }
}
