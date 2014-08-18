#include <png.h>
#include <string>
#include <cstdio>
#include <iostream>
#include <vector>
#include <cstring>
#include "timer.hpp"
#include "tinia_png.hpp"
#include "libjpeg_turbo_wrap.hpp"
#include "homebrew_png.hpp"




int
main(int argc, char **argv)
{
    
    for(int i=1; i<argc; i++) {
        std::string arg( argv[i] );
        if( arg.substr(0,2) == "--" ) {
            // option
        }
        else {
            int w = 0;
            int h = 0;
            std::vector<char> image;
            
            {
                TimeStamp start;
                
                FILE* fp = fopen( arg.c_str(), "rb" );
                if( fp == NULL ) {
                    std::cerr << "Failed to open '" << arg << "'\n";
                    return -1;
                }
                
                std::vector<unsigned char> header(8);
                if( fread( header.data(), 1, header.size(), fp ) != header.size() ) {
                    std::cerr << "Error loading file header of '" << arg << "'\n";
                    return -1;
                }
                
                if( png_sig_cmp( header.data(), 0, header.size() ) != 0 ) {
                    std::cerr << "File '" << arg << "' is not a valid PNG file.\n";
                    return -1;
                }
                
                png_structp png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
                if( png_ptr == NULL ) {
                    std::cerr << "Failed to create png_struct.\n";
                    return -1;
                }
                
                png_infop info_ptr = png_create_info_struct( png_ptr );
                if( info_ptr == NULL ) {
                    std::cerr << "Failed to create info struct.\n";
                    return -1;
                }
                
                png_infop end_info = png_create_info_struct( png_ptr );
                if( end_info == NULL ) {
                    std::cerr << "Failed to create end info.\n";
                    return -1;
                }
                
                if( setjmp( png_jmpbuf(png_ptr) ) ) {
                    std::cerr << "setjmp failed.\n";
                    return -1;
                }
                
                png_init_io( png_ptr, fp );
                png_set_sig_bytes( png_ptr, header.size() );
                png_read_png( png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, NULL );
                
                if( png_get_color_type( png_ptr, info_ptr ) != PNG_COLOR_TYPE_RGB ) {
                    std::cerr << "Source image is not RGB.\n";
                    return -1;
                }
                if( png_get_bit_depth( png_ptr, info_ptr ) != 8 ) {
                    std::cerr << "Bit depth is not 8 bits\n.";
                    return -1;
                }
                
                
                w = png_get_image_width( png_ptr, info_ptr );
                h = png_get_image_height( png_ptr, info_ptr );
                png_bytepp rows = png_get_rows( png_ptr, info_ptr );
                image.resize( 3*w*h );
                
                for( int j=0; j<h; j++ ) {
                    for( int i=0; i<w; i++ ) {
                        memcpy( image.data() + 3*w*j, rows[j], 3*i );
                    }
                }
                TimeStamp stop;
                
                std::cerr << "Read [" << w<< 'x' << h << "] RGB pixels ("<< (3*w*h) << " bytes), " << TimeStamp::delta( start, stop ) << "\n";
            }
             
            create_crc_table();
            {
                double seconds_in_zlib;
                TimeStamp start;
                int bytes = tinia_png( seconds_in_zlib, image, w, h, -1 );
                TimeStamp stop;
                std::cerr << "tinia_png:\t" << TimeStamp::delta( start, stop ) << " (" << seconds_in_zlib << "s in zlib)"  << " ("<< bytes << " bytes)\n";
            }
            {
                double seconds_in_zlib;
                TimeStamp start;
                int bytes = tinia_png( seconds_in_zlib, image, w, h, 0 );
                TimeStamp stop;
                std::cerr << "tinia_png 0:\t" << TimeStamp::delta( start, stop ) << " (" << seconds_in_zlib << "s in zlib)"  << " ("<< bytes << " bytes)\n";
            }
            {
                double seconds_in_zlib;
                TimeStamp start;
                int bytes = tinia_png( seconds_in_zlib, image, w, h, 1 );
                TimeStamp stop;
                std::cerr << "tinia_png 1:\t" << TimeStamp::delta( start, stop ) << " (" << seconds_in_zlib << "s in zlib)"  << " ("<< bytes << " bytes)\n";
            }
            {
                double seconds_in_zlib;
                TimeStamp start;
                int bytes = tinia_png( seconds_in_zlib, image, w, h, 2 );
                TimeStamp stop;
                std::cerr << "tinia_png 2:\t" << TimeStamp::delta( start, stop ) << " (" << seconds_in_zlib << "s in zlib)"  << " ("<< bytes << " bytes)\n";
            }
            {
                double seconds_in_zlib;
                TimeStamp start;
                int bytes = tinia_png( seconds_in_zlib, image, w, h, 3 );
                TimeStamp stop;
                std::cerr << "tinia_png 3:\t" << TimeStamp::delta( start, stop ) << " (" << seconds_in_zlib << "s in zlib)"  << " ("<< bytes << " bytes)\n";
            }
            {
                double seconds_in_zlib;
                TimeStamp start;
                int bytes = tinia_png( seconds_in_zlib, image, w, h, 4 );
                TimeStamp stop;
                std::cerr << "tinia_png 4:\t" << TimeStamp::delta( start, stop ) << " (" << seconds_in_zlib << "s in zlib)" <<" ("<< bytes << " bytes)\n";
            }

            createCRCTable();
            {
                TimeStamp start;
                int bytes = homebrew_png2( image, w, h );
                TimeStamp stop;
                std::cerr << "homebrew_wrap2:\t" << TimeStamp::delta( start, stop ) << " ("<< bytes << " bytes)\n";
            }
            {
                TimeStamp start;
                int bytes = homebrew_png3( image, w, h );
                TimeStamp stop;
                std::cerr << "homebrew_wrap3:\t" << TimeStamp::delta( start, stop ) << " ("<< bytes << " bytes)\n";
            }
            {
                TimeStamp start;
                int bytes = homebrew_png4( image, w, h );
                TimeStamp stop;
                std::cerr << "homebrew_wrap4:\t" << TimeStamp::delta( start, stop ) << " ("<< bytes << " bytes)\n";
            }

            {
                TimeStamp start;
                int bytes = libjpeg_turbo_wrap( image, w, h );
                TimeStamp stop;
                std::cerr << "libjpeg_turbo_wrap:\t" << TimeStamp::delta( start, stop ) << " ("<< bytes << " bytes)\n";
            }
            
        }
        
    }
    
    
    
    
    return 0;
}
