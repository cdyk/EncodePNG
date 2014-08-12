#pragma once
#include <vector>

void
create_crc_table();

bool
tinia_png( const std::vector<char> &rgb,
              const int w,
              const int h );
