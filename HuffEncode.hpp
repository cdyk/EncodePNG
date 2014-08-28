#pragma once
#include <vector>

void
encodeHuffman( std::vector<unsigned char>& output,
               unsigned int** code_stream_p,
               unsigned int*  code_stream_N,
               unsigned int   code_streams );
