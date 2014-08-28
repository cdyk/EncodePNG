#include <xmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>
#include <tmmintrin.h>
#include "ScanlineFilter.hpp"

void
filterScanlinesSSE( unsigned char* filtered,
                    unsigned char* image,
                    unsigned int WIDTH,
                    unsigned int HEIGHT )
{
    int blocks = 3*WIDTH/16;

    // Create move-mask for last block of each scanline
    __m128i mask = _mm_cmplt_epi8( _mm_set_epi8( 15, 14, 13, 12, 11, 10, 9, 8,
                                                  7,  6,  5,  4,  3,  2, 1, 0 ),
                                   _mm_set1_epi8( 3*WIDTH-16*blocks ) );
    {
        const unsigned char* in = image;
        unsigned char* out = filtered;
        *out++ = 0;
        for(int b=0; b<blocks; b++ ) {
            _mm_storeu_si128( (__m128i*)out, _mm_lddqu_si128( (__m128i const*)in ) );
            in += 16;
            out += 16;
        }
        _mm_maskmoveu_si128( _mm_lddqu_si128( (__m128i const*)in ), mask, (char*)out );
    }

    for( unsigned int j=1; j<HEIGHT; j++ ) {
        const unsigned char* in = image + 3*WIDTH*(j-1);
        unsigned char* out = filtered + (3*WIDTH+1)*j;
        *out++ = 2;
        for(int b=0; b<blocks; b++ ) {
            __m128i _t0 = _mm_lddqu_si128( (__m128i const*)in );
            __m128i _t1 = _mm_lddqu_si128( (__m128i const*)(in + 3*WIDTH ) );

            _mm_storeu_si128( (__m128i*)out,
                              _mm_sub_epi8( _t1, _t0 ) );
            in += 16;
            out += 16;
        }
        _mm_maskmoveu_si128( _mm_lddqu_si128( (__m128i const*)in ),
                             mask,
                             (char*)out );

    }
}

void
filterScanlines( unsigned char* filtered,
                 unsigned char* image,
                 unsigned int WIDTH,
                 unsigned int HEIGHT )
{
    for( unsigned int j=0; j<HEIGHT; j++) {
        const unsigned char* in = image + 3*WIDTH*j;
        unsigned char* out = filtered + (3*WIDTH+1)*j;
        *out++ = 1;       // Filter type 1 (diff with left);
        
        unsigned int p0 = 0;
        unsigned int p1 = 0;
        unsigned int p2 = 0;
        for( unsigned int i=0; i<WIDTH; i++ ) {
            unsigned int c0 = *in++;
            unsigned int c1 = *in++;
            unsigned int c2 = *in++;
            unsigned int v0 = (c0 - p0) & 0xffu;
            unsigned int v1 = (c1 - p1) & 0xffu;
            unsigned int v2 = (c2 - p2) & 0xffu;
            p0 = c0;
            p1 = c1;
            p2 = c2;
            *out++ = v0;
            *out++ = v1;
            *out++ = v2;
        }
    }
}