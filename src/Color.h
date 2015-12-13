// -*-C++-*-
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
#ifndef COLOR_H
#define COLOR_H

/** @brief An RGB color */
struct Color {
  /** @brief Constructor
   * @param r Red component
   * @param g Green component
   * @param b Blue component
   *
   * All components have values in the closed interval [0,1].
   */
  Color(double r, double g, double b):
    red(r), green(g), blue(b) {
  }

  /** @brief Constructor
   * @param rgb 24-bit red/green/blue color
   *
   *
   */
  Color(unsigned rgb = 0): red(component(rgb, 16)),
                           green(component(rgb, 8)),
                           blue(component(rgb, 0)) {
  }

  operator unsigned() const {
    return pack(red, 16) + pack(green, 8) + pack(blue, 0);
  }

  /** @brief Red component */
  double red;

  /** @brief Green component */
  double green;

  /** @brief Blue component */
  double blue;

private:
  static double component(unsigned n, unsigned shift) {
    return (double)((n >> shift) & 255) / 255.0;
  }

  static unsigned pack(double c, unsigned shift) {
    return static_cast<unsigned>(255.0 * c) << shift;
  }
};

#endif /* COLOR_H */
