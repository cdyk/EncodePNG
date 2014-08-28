#pragma once

void
filterScanlinesSSE( unsigned char* filtered,
                    unsigned char* image,
                    unsigned int WIDTH,
                    unsigned int HEIGHT );
void
filterScanlines( unsigned char* filtered,
                 unsigned char* image,
                 unsigned int WIDTH,
                 unsigned int HEIGHT );
