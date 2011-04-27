#include <config.h>
#include "Conf.h"

void Volume::select(bool sense) {
  isSelected = sense;
}

bool Volume::valid(const std::string &name) {
  return name.find_first_not_of(VOLUME_VALID) == std::string::npos;
}

void Volume::calculate() {
  completed = 0;
  for(std::set<Backup>::const_iterator it = backups.begin();
      it != backups.end();
      ++it) {
    const Backup &s = *it;
    // Only count complete backups
    if(s.rc == 0) {
      // Global figures
      ++completed;
      if(completed == 1 || s.date < oldest)
        oldest = s.date;
      if(completed == 1 || s.date > newest)
        newest = s.date;

      // Per-device figures
      Volume::PerDevice &pd = perDevice[s.deviceName];
      ++pd.count;
      if(pd.count == 1 || s.date < pd.oldest)
        pd.oldest = s.date;
      if(pd.count == 1 || s.date > pd.newest)
        pd.newest = s.date;
    }
  }
}

void Volume::addBackup(const Backup &backup) {
  backups.insert(backup);
  calculate();
}

bool Volume::removeBackup(const Backup *backup) {
  for(std::set<Backup>::const_iterator it = backups.begin();
      it != backups.end();
      ++it) {
    const Backup &s = *it;
    if(&s == backup) {
      backups.erase(it);
      // Recalculate totals
      calculate();
      return true;
    }
  }
  return false;
}
