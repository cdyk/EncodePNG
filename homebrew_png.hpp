#pragma once
#include "ThreadPool.hpp"

void
createCRCTable( );

int
homebrew_png2( const std::vector<char> &rgb,
              const int w,
              const int h );

int
homebrew_png3( const std::vector<char> &rgb,
              const int w,
              const int h );

int
homebrew_png4( ThreadPool* thread_pool,
               const std::vector<char> &rgb,
              const int w,
              const int h );

int
homebrew_png4_mc( ThreadPool* thread_pool,
                  const std::vector<char> &rgb,
                  const int w,
                  const int h );
