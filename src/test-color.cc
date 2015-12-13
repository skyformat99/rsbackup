// Copyright © 2015 Richard Kettlewell.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
#include <config.h>
#include "Color.h"
#include <cassert>
#include <cstdio>

int main() {
  for(unsigned n = 0; n < 256; ++n) {
    Color c(n);
    unsigned u = n;
    printf("%s:%d: c=(%g,%g,%g) n=%u u=%u\n", __FILE__, __LINE__,
           c.red, c.green, c.blue, n, u);
    assert(u == n);
    c = Color(n << 8);
    u = c >> 8;
    printf("%s:%d: c=(%g,%g,%g) n=%u u=%u\n", __FILE__, __LINE__,
           c.red, c.green, c.blue, n, u);
    assert(u == n);
    c = Color(n << 16);
    u = c >> 16;
    printf("%s:%d: c=(%g,%g,%g) n=%u u=%u\n", __FILE__, __LINE__,
           c.red, c.green, c.blue, n, u);
    assert(u == n);
  }
  return 0;
}
