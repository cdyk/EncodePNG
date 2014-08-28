#include <sys/types.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>
#include <tmmintrin.h>

unsigned int
computeAdler32SSE( unsigned char* data,
                   unsigned int N )
{
    unsigned int s1 = 1;
    unsigned int s2 = 0;
    unsigned int pre = (0x10 - ((size_t)(data)&0xfu))&0xfu;
    for(unsigned int i=0; i<pre; i++ ) {
        s1 = s1 + data[i];
        s2 = s2 + s1;
    }
    
    int blocks = (N-pre)/16;
    __m128i Z0 = _mm_set_epi32( 1, 2, 3, 4 );
    __m128i Z1 = _mm_set1_epi32( 4 );
    __m128i Z2 = _mm_set1_epi32( 65521 );
    __m128i Z3 = _mm_set1_epi32( 2*65521 );
    __m128i Z4 = _mm_set1_epi32( 4*65521 );
    __m128i Z5 = _mm_set1_epi32( 8*65521 );
    __m128i Z6 = _mm_set1_epi32( 16*65521 );
    __m128i _tw = _mm_setzero_si128();
    __m128i _tn = _mm_setzero_si128();        
    for(int b=0; b<blocks; b++) {
        __m128i v = _mm_load_si128( (__m128i const*)(data + pre + 16*b) );

        __m128i s0 = _mm_unpacklo_epi8( v, _mm_setzero_si128() );
        __m128i w0 = _mm_unpacklo_epi16( s0, _mm_setzero_si128() );
        _tw = _mm_add_epi32( _mm_mullo_epi32( Z1, _tn ), _mm_add_epi32( _tw, _mm_mullo_epi32( Z0, w0 ) ) );
        _tn = _mm_add_epi32( _tn, w0 );

        __m128i w1 = _mm_unpackhi_epi16( s0, _mm_setzero_si128() );
        _tw = _mm_add_epi32( _mm_mullo_epi32( Z1, _tn ), _mm_add_epi32( _tw, _mm_mullo_epi32( Z0, w1 ) ) );
        _tn = _mm_add_epi32( _tn, w1 );

        __m128i s1 = _mm_unpackhi_epi8( v, _mm_setzero_si128() );
        __m128i w2 = _mm_unpacklo_epi16( s1, _mm_setzero_si128() );
        _tw = _mm_add_epi32( _mm_mullo_epi32( Z1, _tn ), _mm_add_epi32( _tw, _mm_mullo_epi32( Z0, w2 ) ) );
        _tn = _mm_add_epi32( _tn, w2 );
        
        __m128i w3 = _mm_unpackhi_epi16( s1, _mm_setzero_si128() );
        _tw = _mm_add_epi32( _mm_mullo_epi32( Z1, _tn ), _mm_add_epi32( _tw, _mm_mullo_epi32( Z0, w3 ) ) );
        _tn = _mm_add_epi32( _tn, w3 );


        // _tw = tw % 65521; _tn = tn % 65521
        _tw = _mm_sub_epi32( _tw,_mm_andnot_si128( _mm_cmplt_epi32( _tw, Z6 ), Z6 ) );
        _tw = _mm_sub_epi32( _tw,_mm_andnot_si128( _mm_cmplt_epi32( _tw, Z5 ), Z5 ) );
        _tw = _mm_sub_epi32( _tw,_mm_andnot_si128( _mm_cmplt_epi32( _tw, Z4 ), Z4 ) );
        _tw = _mm_sub_epi32( _tw,_mm_andnot_si128( _mm_cmplt_epi32( _tw, Z3 ), Z3 ) );
        _tw = _mm_sub_epi32( _tw,_mm_andnot_si128( _mm_cmplt_epi32( _tw, Z2 ), Z2 ) );

        _tn = _mm_sub_epi32( _tn,_mm_andnot_si128( _mm_cmplt_epi32( _tn, Z2 ), Z2 ) );
    }
    // 4:1 reduction
    _tw = _mm_hadd_epi32( _tw, _mm_setzero_si128() );
    _tw = _mm_hadd_epi32( _tw, _mm_setzero_si128() );

    _tn = _mm_hadd_epi32( _tn, _mm_setzero_si128() );
    _tn = _mm_hadd_epi32( _tn, _mm_setzero_si128() );
    
    s2 = s2 + 16*blocks*s1 + _mm_cvtsi128_si32( _tw );
    s1 = s1 + _mm_cvtsi128_si32( _tn );

    for(unsigned int i=pre+16*blocks; i<N; i++ ) {
        s1 = s1 + data[i];
        s2 = s2 + s1;
    }
    return ((s2%65521) << 16) + (s1%65521);
}