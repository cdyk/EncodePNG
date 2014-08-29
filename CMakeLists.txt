PROJECT( imgcompbench )
CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )

SET( CMAKE_CXX_FLAGS "-Wall -O3 -msse4 -g2 -DDEBUG -DUNIX -std=gnu++0x ${CMAKE_CXX_FLAGS}" )
FIND_PACKAGE( ZLIB REQUIRED )
FIND_PACKAGE( PNG REQUIRED )
FIND_LIBRARY( JPEG_TURBO_LIBRARIES NAMES jpeg )
ADD_EXECUTABLE( main "main.cpp"
                "ThreadPool.hpp"
                "ThreadPool.cpp"
                "HuffEncode.hpp"
                "HuffEncode.cpp"
                "LZEncoder.hpp"
                "LZEncoder.cpp"
                "ScanlineFilter.hpp"
                "ScanlineFilter.cpp"
                "Adler32.hpp"
                "Adler32.cpp"
                "BitPusher.hpp"
                "tinia_png.hpp"
                "tinia_png.cpp"
                "timer.hpp"
                "timer.cpp"
                "homebrew_png.hpp"
                "homebrew_png.cpp"
                "libjpeg_turbo_wrap.hpp"
                "libjpeg_turbo_wrap.cpp")
TARGET_LINK_LIBRARIES( main  ${JPEG_TURBO_LIBRARIES} ${PNG_LIBRARIES} ${ZLIB_LIBRARIES} rt pthread )
