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

ColorStrategy::ColorStrategy(const char *name): name(name) {
  if(!strategies)
    strategies = new strategies_type();
  (*strategies)[name] = this;
}

std::string ColorStrategy::description() const {
  return name;
}

const ColorStrategy *ColorStrategy::find(const std::string &name) {
  if(strategies == nullptr)
    throw std::logic_error("ColorStrategy::find");
  auto it = strategies->find(name);
  return it != strategies->end() ? it->second : nullptr;
}

/** @brief A color strategy that maximizes distance between hues */
static class EquidistantHue: public ColorStrategy {
public:
  EquidistantHue(): ColorStrategy("equidistant-hue") {
  }

  Color get(unsigned n, unsigned items) const override {
    double h = 360.0 * n / items;
    return Color::HSV(h, 1, 1);
  }
} equidistant_hue;

/** @brief A color strategy that maximizes distance between values */
static class EquidistantValue: public ColorStrategy {
public:
  EquidistantValue(): ColorStrategy("equidistant-value") {
  }

  Color get(unsigned n, unsigned items) const override {
    double v = static_cast<double>(n) / (items-1);
    double h = 0;                       // TODO should be configurable
    return Color::HSV(h, 1, v);
  }
} equidistant_value;

ColorStrategy::strategies_type *ColorStrategy::strategies;

const ColorStrategy *const ColorStrategy::defaultStrategy = &equidistant_hue;
