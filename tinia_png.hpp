#pragma once
#include <vector>

void
create_crc_table();

int
tinia_png( double& seconds_in_zlib,
           const std::vector<char> &rgb,
              const int w,
              const int h , int compression);
