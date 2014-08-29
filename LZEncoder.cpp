#include <vector>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>
#include <tmmintrin.h>
#include "LZEncoder.hpp"


static
int
lengthOfMatch( const unsigned char* a,
               const unsigned char* b,
               const int N )
{

#if 1
    for(int i=0; i<N; i+=16 ) {
        __m128i _a = _mm_lddqu_si128( (__m128i const*)(a+i) );
        __m128i _b = _mm_lddqu_si128( (__m128i const*)(b+i) );
        unsigned int q = _mm_movemask_epi8( _mm_cmpeq_epi8( _a, _b ) );
        if( q != 0xffffu ) {
            int l = i + __builtin_ctz( ~q );
            return std::min( N, l );
        }
    }
#else
    for( int i=0; i<N; i++ ) {
        if( *a++ != *b++ ) {
            return i;
        }
    }
#endif
    return N;
}


unsigned int
encodeLZ( unsigned int* code_stream,
          unsigned char* data,
          unsigned int N )
{
    int head[256];
    for(int i=0; i<256; i++) {
        head[i] = -0xfffff;
    }
    
    int next[0x10000];
//    for(int i=0; i<0x10000; i++) {
//        next[i] = -0xfffff;
//    }
    

    //std::vector<int> head(256, -0xfffff);
    //std::vector<int> next( 0x10000, -0xfffff );
    
    unsigned int* p = code_stream;
    int i=0;
    while( i < N ) {
        unsigned int h = (13*(13*data[i] + data[i+1])+data[i+2])&0xffu;
        int j = head[h];
        next[ i & 0x7fff ] = j;
        head[h] = i;
        int b_l = 0;
        int b_j = 0;
        for( int k=0; k<10 && (i-j <= 0x7fff); k++ ) {
            int l = lengthOfMatch( data + j,
                                   data + i,
                                   std::min( std::min( 258, (int)N-i), i-j ) );

            if( l > b_l ) {
                b_l = l;
                b_j = j;
                if( b_l == 258 ) {
                    break;// we can't get a longer match.
                }
            }
            j = next[ j & 0x7fff ];
        }
        if( b_l < 3 ) {
             // No matches found, emit literal
            *p++ = 0x80000000u | data[i];
            i++;
        }
        else {
            // emit length-distance pair
            *p++ = ((unsigned int)b_l << 16u) | (i - b_j);
            i = i + b_l;
        }
    }
    return p-code_stream;
}