#pragma once

unsigned int
computeAdler32( unsigned char* data,
                unsigned int N );

unsigned int
computeAdler32Blocked( unsigned char* data,
                       unsigned int N );

unsigned int
computeAdler32SSE( unsigned char* data,
                   unsigned int N );
