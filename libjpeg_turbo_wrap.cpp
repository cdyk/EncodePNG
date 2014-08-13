#include <iostream>
#include <jpeglib.h>
#include <cstdio>
#include "libjpeg_turbo_wrap.hpp"

int
libjpeg_turbo_wrap( const std::vector<char>& rgb,
                    const int w,
                    const int h )
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    
    FILE* of = fopen( "output.jpg", "wb" );
    if( of == NULL ) {
        std::cerr << "Cannot open 'output.jpg'\n";
        return 0;
    }
    jpeg_stdio_dest( &cinfo, of );

    cinfo.image_width = w;
    cinfo.image_height = h;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&cinfo);
    

    std::vector<unsigned char*> rows( h );
    for(int j=0; j<h; j++ ) {
        rows[ j ] = (unsigned char*)(rgb.data() + 3*w*j);
    }

    jpeg_start_compress(&cinfo, TRUE);
    jpeg_write_scanlines( &cinfo, rows.data(), h );
    jpeg_finish_compress(&cinfo);
    
    int bytes = ftell( of );
    
    fclose(of);
    jpeg_destroy_compress(&cinfo);
    return bytes;
}
