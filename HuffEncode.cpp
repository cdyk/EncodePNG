#include "HuffEncode.hpp"
#include "BitPusher.hpp"

void
encodeHuffman( std::vector<unsigned char>& output,
               unsigned int** code_stream_p,
               unsigned int*  code_stream_N,
               unsigned int   code_streams )
{
    
    output.push_back(  8 + (7<<4) );  // CM=8=deflate, CINFO=7=32K window size = 112
    output.push_back( 94 /* 28*/ );           // FLG

    BitPusher pusher( output );
    pusher.pushBits( 1, 1 );    // BFINAL
    pusher.pushBits( 1, 2 );    // BTYPE (=01)

    for( unsigned int k=0; k<code_streams; k++ ) {
        unsigned int* codes = code_stream_p[k];
        for( unsigned int j=0; j<code_stream_N[k]; j++ ) {
            unsigned int code = *codes++;
            
            // --- Literal value -----------------------------------------------
            // max 9 bits
            if( code&0x80000000u ) {
                code = code&0xffu;
                if( code < 144 ) {
                    pusher.pushBitsReverse( code + 48, 8 );
                }
                else {
                    pusher.pushBitsReverse( code + (400-144), 9 );
                }
            }
            
            // --- Length-distance pair ----------------------------------------
            else {
                unsigned int length   = code >> 16u;
                unsigned int distance = code & 0xffffu;
                
                // --- 7-bit length Huffman code -------------------------------
                if( length < 115 ) {    // 7-bit length Huffman code
                    unsigned int length_code, length_bits, length_bits_n;
                    
                    if( length < 11 ) {
                        length_code     = length-2;
                        length_bits     = 0;
                        length_bits_n   = 0;
                    }
                    else if(length < 19 ) {
                        length_code     = ((length-11)>>1)+9;
                        length_bits     = (length-11)&0x1;
                        length_bits_n   = 1;
                    }
                    else if(length < 35 ) {
                        length_code     = ((length-19)>>2)+13;
                        length_bits     = (length-19)&0x3;
                        length_bits_n   = 2;
                    }
                    else if(length < 67 ) {
                        length_code     = ((length-35)>>3)+(273-256);
                        length_bits     = (length-35)&0x7;
                        length_bits_n   = 3;
                    }
                    else {  // length < 131
                        length_code     = ((length-67)>>4)+(277-256);
                        length_bits     = (length-67)&0xf;
                        length_bits_n   = 4;
                    }
                    length_code = ((length_code&0x55u)<<1u) | ((length_code>>1u)&0x55u);
                    length_code = ((length_code&0x33u)<<2u) | ((length_code>>2u)&0x33u);
                    length_code = ((length_code&0x0fu)<<4u) | ((length_code>>4u)&0x0fu);
                    length_code = (length_code>>1);
                    length_code = length_code | (length_bits<<7);
                    pusher.pushBits( length_code, 7 + length_bits_n );
                }
                else if( length < 258 ) {                  // 8-bit length Huffman code
                    unsigned int length_code, length_bits, length_bits_n;
                    
                    if( length < 131 ) {
                        length_code     = 192;
                        length_bits     = (length-115)&0xf;
                        length_bits_n   = 4;
                    }
                    else if( length < 258 ) {
                        length_code     = ((length-131)>>5)+(281-280+192);
                        length_bits     = (length-131)&0x1f;
                        length_bits_n   = 5;
                    }
                    else {
                        length_code     = (285-280+192);
                        length_bits     = 0;
                        length_bits_n   = 0;
                    }
                    
                    length_code = ((length_code&0x55u)<<1u) | ((length_code>>1u)&0x55u);
                    length_code = ((length_code&0x33u)<<2u) | ((length_code>>2u)&0x33u);
                    length_code = ((length_code&0x0fu)<<4u) | ((length_code>>4u)&0x0fu);
                    length_code = length_code | (length_bits<<8);
                    pusher.pushBits( length_code, 8 + length_bits_n );
                    
                }
                else {
                    
                    pusher.pushBits( 163, 8 );  // = 197 reversed.
                }
                
                // --- Encode distance Huffman codes ---------------------------
                
                if( distance < 5 ) {
                    unsigned int distance_code;
                    distance_code   = distance-1;   // 
                    
                    distance_code = ((distance_code&0x55u)<<1u) | ((distance_code>>1u)&0x55u);
                    distance_code = ((distance_code&0x33u)<<2u) | ((distance_code>>2u)&0x33u);
                    distance_code = ((distance_code&0x0Fu)<<1u) | ((distance_code>>7u)&0x01u);
                    pusher.pushBits( distance_code, 5u );
                }
                else {
                    unsigned int distance_code = 0;
                    unsigned int distance_bits = 0;
                    unsigned int distance_bits_n = 0;
                    
                    for(unsigned int i=1; i<14u; i++ ) {
                        if( distance < ((4u<<i)+1u) ) {
                            distance_code   = ((distance - ((4<<(i-1))+1))>>i) + (2+2*i);
                            distance_bits   = (distance - ((4<<(i-1))+1)) & ((1<<i)-1);
                            distance_bits_n = i;
                            break;
                        }
                    }
                    
                    distance_code = ((distance_code&0x55u)<<1u) | ((distance_code>>1u)&0x55u);
                    distance_code = ((distance_code&0x33u)<<2u) | ((distance_code>>2u)&0x33u);
                    distance_code = ((distance_code&0x0Fu)<<1u) | ((distance_code>>7u)&0x01u);
                    
                    distance_code = distance_code | (distance_bits<<5u);
                    pusher.pushBits( distance_code, 5u + distance_bits_n );
                }
                
            }
        }
    }
    pusher.pushBits( 0, 7 );    // EOB
}