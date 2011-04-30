// Copyright © 2011 Richard Kettlewell.
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
#include "Date.h"
#include "Errors.h"
#include <cstdio>
#include <cstdlib>

// Cumulative day numbers at start of each month
// (for a non-leap-year)
const int Date::mday[] = {
  0,
  0,                                    // January
  31,
  31 + 28,
  31 + 28 + 31,
  31 + 28 + 31 + 30,
  31 + 28 + 31 + 30 + 31,
  31 + 28 + 31 + 30 + 31 + 30,
  31 + 28 + 31 + 30 + 31 + 30 + 31,
  31 + 28 + 31 + 30 + 31 + 30 + 31 + 31,
  31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30,
  31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,
  31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30,
  31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + 31,
};

Date::Date(const std::string &dateString) {
  if(sscanf(dateString.c_str(), "%d-%d-%d", &y, &m, &d) != 3)
    throw InvalidDate("invalid date string '" + dateString + "'");
  if(y < 1)
    throw InvalidDate("invalid date string '" + dateString + "' - year too small");
  if(m < 1 || m > 12)
    throw InvalidDate("invalid date string '" + dateString + "' - month out of range"); 
  if(d < 1 || d > monthLength(y, m))
    throw InvalidDate("invalid date string '" + dateString + "' - day out of range"); 
}

Date::Date(time_t when) {
  struct tm result;
  localtime_r(&when, &result);
  y = result.tm_year + 1900;
  m = result.tm_mon + 1;
  d = result.tm_mday;
}

std::string Date::toString() const {
  char buffer[64];
  snprintf(buffer, sizeof buffer, "%04d-%02d-%02d", y, m, d);
  return buffer;
}

int Date::toNumber() const {
  int dayno = 365 * y + y / 4 - y / 100 + y / 400;
  dayno += mday[m] + (m > 2 && isLeapYear() ? 1 : 0);
  dayno += d - 1;
  return dayno;
}

bool Date::operator<(const Date &that) const {
  int delta;
  if((delta = y - that.y)) return delta < 0;
  if((delta = m - that.m)) return delta < 0;
  if((delta = d - that.d)) return delta < 0;
  return false;
}

int Date::operator-(const Date &that) const {
  return toNumber() - that.toNumber();
}

Date Date::today() {
  // Allow overriding of 'today' form environment for testing
  const char *override = getenv("RSBACKUP_TODAY");
  if(override)
    return Date(override);
  time_t now;
  time(&now);
  return Date(now);
}

int Date::monthLength(int y, int m) {
  int len = mday[m + 1] - mday[m];
  if(m == 2 && isLeapYear(y))
    ++len;
  return len;
}
