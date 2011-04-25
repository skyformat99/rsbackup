#include <config.h>
#include "rsbackup.h"
#include "Conf.h"
#include "Command.h"
#include "Errors.h"
#include "IO.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// Decide which old backups to prune.
void pruneOld() {
  // Make sure all state is available
  config.readState();
  
  // Figure out which backups are obsolete, if any
  std::vector<const Status *> oldBackups;
  for(hosts_type::iterator hostsIterator = config.hosts.begin();
      hostsIterator != config.hosts.end();
      ++hostsIterator) {
    Host *host = hostsIterator->second;
    for(volumes_type::iterator volumesIterator = host->volumes.begin();
        volumesIterator != host->volumes.end();
        ++volumesIterator) {
      Volume *volume = volumesIterator->second;
      Date today = Date::today();
      for(backups_type::iterator backupsIterator = volume->backups.begin();
          backupsIterator != volume->backups.end();
          ++backupsIterator) {
        const Status &status = *backupsIterator;
        int age = today - status.date;
        // Keep backups that are young enough
        if(age <= volume->pruneAge)
          continue;
        // Keep backups that are on underpopulated devices
        Volume::PerDevice &pd = volume->perDevice[status.deviceName];
        if(pd.count - pd.toBeRemoved <= volume->minBackups)
          continue;
        // Prune whatever's left
        oldBackups.push_back(&status);
        ++pd.toBeRemoved;
      }
    }
  }

  // Return straight away if there's nothing to do
  if(oldBackups.size() == 0)
    return;

  // Identify devices
  config.identifyDevices();

  // Log what we delete  
  StdioFile logFile;
  std::string today = Date::today().toString();
  if(command.act)
    logFile.open(config.logs + PATH_SEP + "prune-" + today + ".log", "a");

  // Delete obsolete backups
  for(size_t n = 0; n < oldBackups.size(); ++n) {
    const Status &status = *oldBackups[n];
    Device *device = config.findDevice(status.deviceName);
    Store *store = device->store;
    // Can't delete backups from unavailable stores
    if(!store)
      continue;
    std::string backupPath = status.backupPath();
    std::string logPath = status.logPath();
    std::string incompletePath = backupPath + ".incomplete";
    if(command.verbose)
      printf("prune %s\n", backupPath.c_str());
    if(command.act) {
      // Mark the backup as 'incomplete' so it will eventually be reaped even
      // if the rm is interrupted
      try {
        int fd = open(incompletePath.c_str(), O_WRONLY|O_CREAT, 0666);
        if(fd < 0)
          throw IOError("creating " + incompletePath);
        close(fd);
        // We remove the logfile
        if(unlink(logPath.c_str()) < 0)
          throw IOError("removing " + logPath);
        // We remove the backup
        std::vector<std::string> cmd;
        cmd.push_back("rm");
        cmd.push_back("-rf");
        cmd.push_back(backupPath);
        int rc = execute(cmd);
        if(rc) {
          char buffer[10];
          sprintf(buffer, "%#x", rc);
          throw IOError("removing " + backupPath + ": rm exited with status "
                        + buffer);
        }
        logFile.writef("%s: removed %s\n",
                       today.c_str(), backupPath.c_str());
      } catch(std::runtime_error &exception) {
        // Log anything that goes wrong
        logFile.writef("%s: FAILED to remove %s: %s\n",
                       today.c_str(), backupPath.c_str(), exception.what());
      }
      logFile.flush();
    }

  }
  
}