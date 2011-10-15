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
#include "Command.h"
#include "Defaults.h"
#include "Errors.h"
#include "Conf.h"
#include "IO.h"
#include <getopt.h>
#include <cstdlib>

// Long-only options
enum {
  RETIRE_DEVICE = 256,
  RETIRE = 257,
  WARN_UNKNOWN = 258,
  WARN_STORE = 259,
  WARN_UNREACHABLE = 261,
  WARN_PARTIAL = 262,
  REPEAT_ERRORS = 263,
  NO_REPEAT_ERRORS = 264,
};

static const struct option options[] = {
  { "help", no_argument, 0, 'h' },
  { "version", no_argument, 0, 'V' },
  { "backup", no_argument, 0, 'b' },
  { "html", required_argument, 0, 'H' },
  { "text", required_argument, 0, 'T' },
  { "email", required_argument, 0, 'e' },
  { "prune", no_argument, 0, 'p' },
  { "prune-incomplete", no_argument, 0, 'P' },
  { "store", required_argument, 0, 's' },
  { "retire-device", no_argument, 0, RETIRE_DEVICE },
  { "retire", no_argument, 0, RETIRE },
  { "config", required_argument, 0, 'c' },
  { "wait", no_argument, 0, 'w' },
  { "force", no_argument, 0, 'f' },
  { "dry-run", no_argument, 0, 'n' },
  { "verbose", no_argument, 0, 'v' },
  { "warn-unknown", no_argument, 0, WARN_UNKNOWN },
  { "warn-store", no_argument, 0, WARN_STORE },
  { "warn-unreachable", no_argument, 0, WARN_UNREACHABLE },
  { "warn-partial", no_argument, 0, WARN_PARTIAL },
  { "errors", no_argument, 0, REPEAT_ERRORS },
  { "no-errors", no_argument, 0, NO_REPEAT_ERRORS },
  { "warn-all", no_argument, 0, 'W' },
  { "debug", no_argument, 0, 'd' },
  { 0, 0, 0, 0 }
};

Command::Command(): backup(false),
                    prune(false),
                    pruneIncomplete(false),
                    retire(false),
                    retireDevice(false),
                    html(NULL),
                    text(NULL),
                    email(NULL),
                    configPath(DEFAULT_CONFIG),
                    wait(false),
                    act(true),
                    force(false),
                    verbose(false),
                    warnUnknown(false),
                    warnStore(false),
                    warnPartial(false),
                    repeatErrorLogs(true),
                    debug(false) {
}

void Command::help() {
  IO::out.writef(
"Usage:\n"
"  rsbackup [OPTIONS] [--] [[-]HOST...] [[-]HOST:VOLUME...]\n"
"  rsbackup --retire [OPTIONS] [--] [HOST...] [HOST:VOLUME...]\n"
"  rsbackup --retire-device [OPTIONS] [--] DEVICES...\n"
"\n"
"At least one action option is required:\n"
"  --backup, -b            Back up selected volumes (default: all)\n"
"  --html, -H PATH         Write an HTML report to PATH\n"
"  --text, -T PATH         Write a text report to PATH\n"
"  --email, -e ADDRESS     Mail HTML report to ADDRESS\n"
"  --prune, -p             Prune old backups of selected volumes (default: all)\n"
"  --prune-incomplete, -P  Prune incomplete backups\n"
"  --retire                Retire volumes (must specify at least one)\n"
"  --retire-device         Retire devices (must specify at least one)\n"
"\n"
"Additional options:\n"
"  --store, -s DIR         Override directory(s) to store backups in\n"
"  --config, -c PATH       Set config file (default: /etc/rsbackup/config)\n"
"  --wait, -w              Wait until running rsbackup finishes\n"
"  --force, -f             Don't prompt when retiring\n"
"  --dry-run, -n           Dry run only\n"
"  --verbose, -v           Verbose output\n"
"  --help, -h              Display usage message\n"
"  --version, -V           Display version number\n"
"\n"
"Warning options:\n"
"  --warn-unknown          Warn about unknown devices/volumes\n"
"  --warn-store            Warn about bad stores/unavailable devices\n"
"  --warn-unreachable      Warn about unreachable hosts\n"
"  --warn-partial          Warn about partial transfers\n"
"  --warn-all, -W          Enable all warnings\n"
"  --no-errors             Don't display rsync errors\n"
                 );
  IO::out.close();
  exit(0);
}

void Command::version() {
  IO::out.writef("%s\n", VERSION);
  IO::out.close();
  exit(0);
}

void Command::parse(int argc, char **argv) {
  int n;

  // Parse options
  while((n = getopt_long(argc, argv, "+hVbH:e:pP:c:wnfvdW", options, 0)) >= 0) {
    switch(n) {
    case 'h': help();
    case 'V': version();
    case 'b': backup = true; break;
    case 'H': html = new std::string(optarg); break;
    case 'T': text = new std::string(optarg); break;
    case 'e': email = new std::string(optarg); break;
    case 'p': prune = true; break;
    case 'P': pruneIncomplete = true; break;
    case 's': stores.push_back(optarg); warnStore = true; break;
    case 'c': configPath = optarg; break;
    case 'w': wait = true; break;
    case 'n': act = false; verbose = true; break;
    case 'f': force = true;
    case 'v': verbose = true; break;
    case 'd': debug = true; break;
    case RETIRE_DEVICE: retireDevice = true; break;
    case RETIRE: retire = true; break;
    case WARN_UNKNOWN: warnUnknown = true; break;
    case WARN_STORE: warnStore = true; break;
    case WARN_UNREACHABLE: warnUnreachable = true; break;
    case WARN_PARTIAL: warnPartial = true; break;
    case REPEAT_ERRORS: repeatErrorLogs = true; break;
    case NO_REPEAT_ERRORS: repeatErrorLogs = false; break;
    case 'W': warnUnknown = warnStore = warnUnreachable = warnPartial = true; break;
    default: exit(1);
    }
  }

  // Various options are incompatible with one another
  if(retire && retireDevice)
    throw CommandError("--retire and --retire-device cannot be used together");
  if(backup && retire)
    throw CommandError("--retire and --backup cannot be used together");
  if(backup && retireDevice)
    throw CommandError("--retire-device and --backup cannot be used together");

  // We have to do *something*
  if(!backup
     && !html
     && !text
     && !email
     && !prune
     && !pruneIncomplete
     && !retireDevice
     && !retire)
    throw CommandError("no action specified");

  if(backup || prune || retire) {
    // Volumes to back up or retire
    if(optind < argc) {
      for(n = optind; n < argc; ++n) {
        // Establish the sense of this entry
        bool sense;
        const char *s = argv[n], *t;
        if(*s == '-' || *s == '!') {
          sense = false;
          ++s;
        } else
          sense = true;
        if((t = strchr(s, ':'))) {
          // A host:volume pair
          selections.push_back(Selection(std::string(s, t - s), t + 1, sense));
        } else {
          // Just a host
          selections.push_back(Selection(s, "*", sense));
        }
      }
    } else {
      if(retire)
        throw CommandError("no volumes specified to retire");
      // No volumes requested = back up/prune everything
      selections.push_back(Selection("*", "*", true));
    }
  }
  if(retireDevice) {
    if(optind >= argc)
      throw CommandError("no devices specified to retire");
    for(n = optind; n < argc; ++n)
      devices.push_back(argv[n]);
  }
}

void Command::selectVolumes() {
  // This is a separate method because it has to be called after the config
  // file is read.
  for(size_t n = 0; n < selections.size(); ++n)
    config.selectVolume(selections[n].host,
                        selections[n].volume,
                        selections[n].sense);
}

Command command;
