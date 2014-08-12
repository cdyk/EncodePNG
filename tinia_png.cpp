/* Copyright STIFTELSEN SINTEF 2012
 * 
 * This file is part of the Tinia Framework.
 * 
 * The Tinia Framework is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * The Tinia Framework is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public License
 * along with the Tinia Framework.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <zlib.h>
#include <iostream>
#include "timer.hpp"
#include "tinia_png.hpp"

static unsigned int crc_table[256];

void
create_crc_table()
{
    for( unsigned int j=0; j<256; j++) {
        unsigned int c = j;
        for( unsigned int i=0; i<8; i++) {
            if( c & 0x1 ) {
                c = 0xedb88320ul ^ (c>>1);
            }
            else {
                c = c>>1;
            }
        }
        crc_table[j] = c;
    }
}
    
static
unsigned int
trell_png_crc( unsigned char* p, size_t length )
{
    size_t i;
    unsigned int crc = 0xffffffffl;
    for(i=0; i<length; i++) {
        unsigned int ix = ( p[i]^crc ) & 0xff;
        crc = crc_table[ix]^((crc>>8));
    }
    return ~crc;
}


bool
tinia_png( const std::vector<char> &rgb,
              const int w,
              const int h )
{
    std::vector<unsigned char> filtered( 3*(w+1)*h );

    for( int j=0; j<h; j++ ) {
        filtered[ 3*(w+1)*j + 0 ] = 0;
        for( int i=0; i<w; i++ ) {
            filtered[ (3*w+1)*j + 1 + 3*i + 0 ] = rgb[ 3*w*(j) + 3*i + 0 ];
            filtered[ (3*w+1)*j + 1 + 3*i + 1 ] = rgb[ 3*w*(j) + 3*i + 1 ];
            filtered[ (3*w+1)*j + 1 + 3*i + 2 ] = rgb[ 3*w*(j) + 3*i + 2 ];
        }
    }

    
    uLong bound = compressBound( (3*w+1)*h );

    std::vector<unsigned char> png( bound + 8 + 25 + 12 + 12 + 12 );
    unsigned char* p = png.data();
    
    // PNG signature, 8 bytes
    *p++ = 137; *p++ = 80; *p++ = 78; *p++ = 71; *p++ = 13; *p++ = 10; *p++ = 26; *p++ = 10;

    // IHDR chunk, 13 + 12 (length, type, crc) = 25 bytes
    *p++ = ((13)>>24)&0xffu;   // chunk length
    *p++ = ((13)>>16)&0xffu;
    *p++ = ((13)>>8)&0xffu;
    *p++ = ((13)>>0)&0xffu;
    *p++ = 'I';                // chunk type
    *p++ = 'H';
    *p++ = 'D';
    *p++ = 'R';
    *p++ = ((w)>>24)&0xffu;    // image width
    *p++ = ((w)>>16)&0xffu;
    *p++ = ((w)>>8)&0xffu;
    *p++ = ((w)>>0)&0xffu;
    *p++ = ((h)>>24)&0xffu;    // image height
    *p++ = ((h)>>16)&0xffu;
    *p++ = ((h)>>8)&0xffu;
    *p++ = ((h)>>0)&0xffu;
    *p++ = 8;                  // 8 bits per channel
    *p++ = 2;                  // RGB triple
    *p++ = 0;
    *p++ = 0;
    *p++ = 0;                  // image not interlaced
    unsigned int crc = trell_png_crc( p-13-4, 13+4 );
    *p++ = ((crc)>>24)&0xffu;    // image width
    *p++ = ((crc)>>16)&0xffu;
    *p++ = ((crc)>>8)&0xffu;
    *p++ = ((crc)>>0)&0xffu;


    // IDAT chunk, 12 + payload bytes in total.

    int c;
    {
        TimeStamp start;
        c = compress( (Bytef*)(p+8), &bound, (Bytef*)filtered.data(), (3*w+1)*h );
        TimeStamp stop;
        std::cerr << "compress: " << TimeStamp::delta( start, stop ) << "\n";
    }

    if( c == Z_MEM_ERROR ) {
        std::cerr << "Z_MEM_ERROR\n";
        return false;
    }
    else if( c == Z_BUF_ERROR ) {
        std::cerr << "Z_BUF_ERROR\n";
        return false;
    }

    *p++ = ((bound)>>24)&0xffu;    // compressed image data size
    *p++ = ((bound)>>16)&0xffu;
    *p++ = ((bound)>>8)&0xffu;
    *p++ = ((bound)>>0)&0xffu;
    *p++ = 'I';                // chunk type
    *p++ = 'D';
    *p++ = 'A';
    *p++ = 'T';

    p += bound;                // skip forward
    crc = trell_png_crc( p-bound-4, bound+4 );
    *p++ = ((crc)>>24)&0xffu;    // CRC
    *p++ = ((crc)>>16)&0xffu;
    *p++ = ((crc)>>8)&0xffu;
    *p++ = ((crc)>>0)&0xffu;    

    // IEND chunk
    *p++ = 0;       // zero payload
    *p++ = 0;
    *p++ = 0;
    *p++ = 0;
    *p++ = 'I';     // ID
    *p++ = 'E';
    *p++ = 'N';
    *p++ = 'D';
    *p++ = 174;     // CRC
    *p++ = 66;
    *p++ = 96;
    *p++ = 130;
    
    FILE* of = fopen( "tinia.png", "wb" );
    fwrite( png.data(), 1, p-png.data(), of );
    fclose( of);

    return true;
}


#if 0


     
         
        unsigned char* png = apr_palloc( encoder_state->r->pool, bound + 8 + 25 + 12 + 12 + 12 );
       
    
        
    
        
    
        
    
        
      
        
        char* datestring = apr_palloc( encoder_state->r->pool, APR_RFC822_DATE_LEN );
        apr_rfc822_date( datestring, apr_time_now() );
        apr_table_setn( encoder_state->r->headers_out, "Last-Modified", datestring );
        apr_table_setn( encoder_state->r->headers_out, "Cache-Control", "no-cache" );
    
        struct apr_bucket_brigade* bb = apr_brigade_create( encoder_state->r->pool,
                                                            encoder_state->r->connection->bucket_alloc );
        if( encoder_state->dispatch_info->m_base64 == 0 ) {
            // Send as plain png image
            ap_set_content_type( encoder_state->r, "image/png" );
            ap_set_content_length( encoder_state->r, p-png );
            APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_immortal_create( (char*)png, p-png, bb->bucket_alloc ) );
        }
        else {
            // Encode png as base64 and send as string
            apr_table_setn( encoder_state->r->headers_out, "Content-Type", "text/plain" );
            ap_set_content_type( encoder_state->r, "text/plain" );
            char* base64 = apr_palloc( encoder_state->r->pool, apr_base64_encode_len( p-png ) );
            int base64_size = apr_base64_encode( base64, (char*)png, p-png );
            // Seems like the zero-byte is included in the string size.
            if( (base64_size > 0) && (base64[base64_size-1] == '\0') ) {
                base64_size--;
            }
            APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_immortal_create( base64, base64_size, bb->bucket_alloc ) );
        }
        APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_eos_create( bb->bucket_alloc ) );
    
        apr_status_t rv = ap_pass_brigade( encoder_state->r->output_filters, bb );
        encoder_state->dispatch_info->m_png_exit = apr_time_now();
    
        if( rv != APR_SUCCESS ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, rv, encoder_state->r,
                           "ap_pass_brigade failed." );
            return -1; // error
        }
    }
    return 0;   // success
/*

    
    request_rec* r = encoder_state->m_r;

    tinia_msg_t* msg = (tinia_msg_t*)buffer;
    if( msg->type == TRELL_MESSAGE_IMAGE ) {
        tinia_msg_image_t* m = (tinia_msg_image_t*)buffer;
        int width = m->width;
        int height = m->height;

        
        char* payload = (char*)buffer + sizeof(*m);

    
        
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, encoder_state->m_r,
                       "%s.pass_reply_png: w=%d, h=%d, l=%d.", encoder_state->m_r->handler,
                       width, height, (int)buffer_bytes );
        return 0;
        
        int i, j;
    
        const unsigned int* crc_table = encoder_state->m_sconf->m_crc_table;
    

        
    
    }
    else {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r, "got reply of type %d.", msg->type );
        return -1; // error
    }
    */
}
    
#endif