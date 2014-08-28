#pragma once
#include <vector>

class BitPusher
{
public:
    BitPusher( std::vector<unsigned char>& data )
        : m_data( data ),
          m_pending_data(0u),
          m_pending_count(0u)
    {}
    
    ~BitPusher()
    {
        while( m_pending_count >= 8u ) {
            m_data.push_back( m_pending_data );
            m_pending_data = m_pending_data >> 8u;
            m_pending_count -= 8u;
        }
        if( m_pending_count ) {
            m_data.push_back( m_pending_data );
        }
    }
    
    void
    pushBits( unsigned long long int bits, unsigned int count )
    {
        m_pending_data = m_pending_data | (bits << m_pending_count);
        m_pending_count += count;
        while( m_pending_count >= 8u ) {
            m_data.push_back( m_pending_data );
            m_pending_data = m_pending_data >> 8u;
            m_pending_count -= 8u;
        }
    }

    void
    pushBitsReverse( unsigned long long int bits, unsigned int count )
    {
        //std::cerr << "pushing " << count << " bits:\t";
        //for(int i=0; i<count; i++) {
        //     std::cerr << ((bits>>(count-i-1))&1);
        //}
        //std::cerr << "\n";
        
        bits = bits << (64-count);
        bits = ((bits & 0x5555555555555555ull)<<1)  | ((bits>> 1) & 0x5555555555555555ull);
        bits = ((bits & 0x3333333333333333ull)<<2)  | ((bits>> 2) & 0x3333333333333333ull);
        bits = ((bits & 0x0F0F0F0F0F0F0F0Full)<<4)  | ((bits>> 4) & 0x0F0F0F0F0F0F0F0Full);
        bits = ((bits & 0x00FF00FF00FF00FFull)<<8)  | ((bits>> 8) & 0x00FF00FF00FF00FFull);
        bits = ((bits & 0x0000FFFF0000FFFFull)<<16) | ((bits>>16) & 0x0000FFFF0000FFFFull);
        bits = ((bits & 0x00000000FFFFFFFFull)<<32) | ((bits>>32) & 0x00000000FFFFFFFFull);
        pushBits( bits, count );
    }
    
    
protected:
    std::vector<unsigned char>& m_data;
    unsigned long long int      m_pending_data;
    unsigned int                m_pending_count;
    
    
};
        
