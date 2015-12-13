//-*-C++-*-
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
#include "HistoryGraph.h"
#include "Conf.h"
#include "Errors.h"
#include <limits>
#include <cassert>

HostLabels::HostLabels(HistoryGraphContext &ctx): Render::Grid(ctx) {
  unsigned row = 0;
  if(config.hosts.size() == 0)
    throw std::runtime_error("no hosts found in configuration");
  for(auto host_iterator: config.hosts) {
    Host *host = host_iterator.second;
    if(!host->selected())
      continue;
    if(host->volumes.size() == 0)
      printf("%s has no volumes!?!\n", host_iterator.first.c_str());
    auto t = new Render::Text(ctx, host_iterator.first,
                              ctx.colors["foreground"]);
    cleanup(t);
    add(t, 0, row);
    for(auto volume_iterator: host->volumes) {
      Volume *volume = volume_iterator.second;
      if(!volume->selected())
        continue;
      ++row;
    }
  }
  set_padding(ctx.xpad, ctx.ypad);
}

VolumeLabels::VolumeLabels(HistoryGraphContext &ctx): Render::Grid(ctx) {
  unsigned row = 0;
  for(auto host_iterator: config.hosts) {
    Host *host = host_iterator.second;
    if(!host->selected())
      continue;
    for(auto volume_iterator: host->volumes) {
      Volume *volume = volume_iterator.second;
      if(!volume->selected())
        continue;
      auto t = new Render::Text(ctx, volume_iterator.first,
                                ctx.colors["foreground"]);
      cleanup(t);
      add(t, 0, row);
      ++row;
    }
  }
  set_padding(ctx.xpad, ctx.ypad);
}

DeviceKey::DeviceKey(HistoryGraphContext &ctx):
  Render::Grid(ctx),
  context(ctx) {
  unsigned row = 0;
  for(auto device_iterator: config.devices) {
    const auto &device = device_iterator.first;
    device_rows[device] = row;
    auto t = new Render::Text(ctx, device,
                              ctx.colors["foreground"]);
    cleanup(t);
    add(t, 0, row);
    auto r = new Render::Rectangle(ctx,
                                   ctx.device_key_indicator_width,
                                   ctx.device_indicator_height,
                                   device_color(row));
    cleanup(r);
    add(r, 1, row, -1, 0);
    ++row;
  }
  set_padding(ctx.xpad, ctx.ypad);
}

const Color DeviceKey::device_color(unsigned row) const {
  char di[64];
  snprintf(di, sizeof di, "device%u", row);
  if(context.colors.find(di) != context.colors.end())
    return context.colors[di];
  return context.color_strategy->get(row, config.devices.size());
}

HistoryGraphContent::HistoryGraphContent(HistoryGraphContext &ctx,
                                         const DeviceKey &device_key):
  Render::Widget(ctx),
  earliest(INT_MAX, 1, 0),
  latest(0),
  context(ctx),
  device_key(device_key),
  rows(0) {
  for(auto host_iterator: config.hosts) {
    Host *host = host_iterator.second;
    if(!host->selected())
      continue;
    for(auto volume_iterator: host->volumes) {
      Volume *volume = volume_iterator.second;
      if(!volume->selected())
        continue;
      for(auto backup: volume->backups) {
        if(backup->getStatus() == COMPLETE) {
          earliest = std::min(earliest, backup->date);
          latest = std::max(latest, backup->date);
        }
      }
      ++rows;
    }
  }
  if(rows == 0)
    throw CommandError("no volumes selected");
}

void HistoryGraphContent::set_extent() {
  assert(row_height > 0);
  auto columns = latest.toNumber() - earliest.toNumber() + 1;
  height = rows ? row_height * rows + context.ypad * (rows - 1) : 0;
  width = latest >= earliest ? context.day_width * columns : 0;
}

void HistoryGraphContent::render_vertical_guides() {
  set_source_color(context.colors["month_guide"]);
  Date d = earliest;
  while(d <= latest) {
    d.d = 1;
    d.addMonth();
    Date next = d;
    next.addMonth();
    double x = (d - earliest) * context.day_width;
    double w = (next - d) * context.day_width;
    w = std::min(w, width - x);
    context.cairo->rectangle(x, 0, w, height);
    d.addMonth();
  }
  context.cairo->fill();
}

void HistoryGraphContent::render_horizontal_guides() {
  unsigned row = 0;
  for(auto host_iterator: config.hosts) {
    Host *host = host_iterator.second;
    if(!host->selected())
      continue;
    set_source_color(context.colors["host_guide"]);
    for(auto volume_iterator: host->volumes) {
      Volume *volume = volume_iterator.second;
      if(!volume->selected())
        continue;
      context.cairo->rectangle(0, row * (row_height + context.ypad),
                               width, 1);
      context.cairo->fill();
      set_source_color(context.colors["volume_guide"]);
      ++row;
    }
  }
  set_source_color(context.colors["volume_guide"]);
  context.cairo->rectangle(0, row * (row_height + context.ypad) - context.ypad,
                           width, 1);
  context.cairo->fill();
}

void HistoryGraphContent::render_data() {
  double y = 0;
  double base = floor((row_height
                       - (context.device_indicator_height
                          * config.devices.size())) / 2);
  for(auto host_iterator: config.hosts) {
    Host *host = host_iterator.second;
    if(!host->selected())
      continue;
    for(auto volume_iterator: host->volumes) {
      Volume *volume = volume_iterator.second;
      if(!volume->selected())
        continue;
      for(auto backup: volume->backups) {
        if(backup->getStatus() == COMPLETE) {
          double x = (backup->date - earliest) * context.day_width;
          auto device_row = device_key.device_row(backup);
          double offset = base + device_row * context.device_indicator_height;
          set_source_color(device_key.device_color(backup));
          context.cairo->rectangle(x, y + offset,
                                   context.day_width,
                                   context.device_indicator_height);
          context.cairo->fill();
        }
      }
      y += row_height + context.ypad;
    }
  }
}

void HistoryGraphContent::render() {
  render_vertical_guides();
  render_horizontal_guides();
  render_data();
}

TimeLabels::TimeLabels(HistoryGraphContext &ctx,
                       HistoryGraphContent &content): Render::Container(ctx),
                                                      context(ctx),
                                                      content(content) {
}

void TimeLabels::set_extent() {
  if(width < 0) {
    Date d = content.earliest;
    int year = -1;
    double limit = 0;
    while(d <= content.latest) {
      Date next = d;
      next.d = 1;
      next.addMonth();
      double xnext = (next - content.earliest) * context.day_width;
      auto t = new Render::Text(context, "",
                                context.colors["foreground"]);
      cleanup(t);
      // Try increasingly compact formats until one fits
      static const char *const formats[] = {
        "%B %Y",
        "%b %Y",
        "%b %y",
        "%B",
        "%b",
      };
      static const unsigned nformats = sizeof formats / sizeof *formats;
      unsigned format;
      double x;
      for(format = (d.y != year ? 0 : 3); format < nformats; ++format) {
        t->set_text(d.format(formats[format]));
        t->set_extent();
        // At the right hand edge, push back so it fits
        x = std::min((d - content.earliest) * context.day_width,
                     content.width - t->width);
        // If it fits, use it
        if(x >= limit && x + t->width < xnext)
          break;
      }
      if(format < nformats) {
        add(t, x, 0);
        limit = x + t->width;
        year = d.y;
      }
      d = next;
    }
    Container::set_extent();
  }
}

HistoryGraph::HistoryGraph(HistoryGraphContext &ctx):
  Render::Grid(ctx),
  context(ctx),
  host_labels(ctx),
  volume_labels(ctx),
  device_key(ctx),
  content(ctx, device_key),
  time_labels(ctx, content) {
  add(&host_labels, 0, 0);
  add(&volume_labels, 1, 0);
  add(&content, 2, 0);
  add(&time_labels, 2, 1);
  add(&device_key, 2, 3, 1, 0);
  set_padding(ctx.xpad, ctx.ypad);
}

void HistoryGraph::set_extent() {
  host_labels.set_extent();
  volume_labels.set_extent();
  device_key.set_extent();
  //time_labels.set_extent();
  double row_height = std::max({host_labels.get_maximum_height(),
        volume_labels.get_maximum_height(),
        (context.device_indicator_height
         * config.devices.size())});
  host_labels.set_minimum(0, row_height);
  volume_labels.set_minimum(0, row_height);
  content.set_row_height(row_height);
  content.set_extent();
  Grid::set_extent();
}

void HistoryGraph::render() {
  set_source_color(context.colors["background"]);
  context.cairo->rectangle(0, 0, width, height);
  context.cairo->fill();
  Grid::render();
}

HistoryGraphContext::HistoryGraphContext() {
  colors["background"] = {1,1,1};
  colors["foreground"] = {0,0,0};
  colors["month_guide"] = {0.96875,0.96875,0.96875};
  colors["volume_guide"] = {0.9375,0.9375,0.9375};
  colors["host_guide"] = {0.875,0.875,0.875};
  color_strategy = ColorStrategy::find("equidistant-hue");
}
