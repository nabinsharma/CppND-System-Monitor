#include <algorithm>
#include <iostream>
#include <math.h>
#include <thread>
#include <chrono>
#include <iterator>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <time.h>
#include <unistd.h>
#include "constants.h"
#include "util.h"

using namespace std;

class ProcessParser{
  private:
    std::ifstream stream;
  public:
    static string getCmd(string pid);
    static vector<string> getPidList();
    static std::string getVmSize(string pid);
    static std::string getCpuPercent(string pid);
    static long int getSysUpTime();
    static std::string getProcUpTime(string pid);
    static string getProcUser(string pid);
    static vector<string> getSysCpuPercent(string coreNumber = "");
    static float getSysRamPercent();
    static string getSysKernelVersion();
    static int getTotalThreads();
    static int getTotalNumberOfProcesses();
    static int getNumberOfRunningProcesses();
    static string getOSName();
    static std::string PrintCpuStats(std::vector<std::string> values1, std::vector<std::string>values2);
    static bool isPidExisting(string pid);
};

// TODO: Define all of the above functions below:
string ProcessParser::getVmSize(std::string pid) {
  string key("VmData");
  ifstream fStatus;
  Util::getStream(Path::basePath() + pid + Path::statusPath(), fStatus);
  string line;
  float memSizeMB = 0;
  while (getline(fStatus, line)) {
    if (line.find(key) != 0)
      continue;
    vector<string> lineSubStrs = Util::streamLineToStringVector(line);
    memSizeMB = stof(lineSubStrs[1]) / 1024;
  }
  return to_string(memSizeMB);
}

string ProcessParser::getProcUpTime(string pid) {
  // Index (1 based) of the entry in stat result (from proc man page)
  int idxInStatResult = 14;
  
  ifstream fStat;
  Util::getStream(Path::basePath() + pid + "/" + Path::statPath(), fStat);
  string result;
  getline(fStat, result);
  vector<string> statEntries = Util::streamLineToStringVector(result);
  return to_string(stof(statEntries[idxInStatResult - 1]) / sysconf(_SC_CLK_TCK));
}

long int ProcessParser::getSysUpTime() {
  ifstream fUptime;
  Util::getStream(Path::basePath() + Path::upTimePath(), fUptime);
  string result;
  getline(fUptime, result);
  vector<string> upTimeEntries = Util::streamLineToStringVector(result);
  // First entry is the system uptime in seconds
  return stoi(upTimeEntries[0]);
}

string ProcessParser::getCpuPercent(string pid) {
  // Indices (1 based) to access various /proc/pid/stat parameters.
  // Based on proc/stat man page.
  int utimeIdx = 14;
  int stimeIdx = 15;
  int cutimeIdx = 16;
  int cstimeIdx = 17;
  int starttimeIdx = 22;
  float freq = sysconf(_SC_CLK_TCK);
  
  ifstream fStat;
  Util::getStream(Path::basePath() + pid + "/" + Path::statPath(), fStat);
  string result;
  getline(fStat, result);
  vector<string> statEntries = Util::streamLineToStringVector(result);
  
  // Could use getProcUpTime, but conversion back to CPU ticks is clumsy.
  float utime = stof(statEntries[utimeIdx - 1]);
  
  float stime = stof(statEntries[stimeIdx - 1]);
  float cutime = stof(statEntries[cutimeIdx - 1]);
  float cstime = stof(statEntries[cstimeIdx - 1]);
  float timeConsumed = (utime + stime + cutime + cstime) / freq;
  
  float starttime = stof(statEntries[starttimeIdx - 1]);
  float uptimeSec = ProcessParser::getSysUpTime();
  float timeElapsed = uptimeSec - (starttime / freq);
  
  return to_string((timeConsumed / timeElapsed) * 100);
}
