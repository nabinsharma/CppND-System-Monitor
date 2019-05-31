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
    static float getSysActiveCpuTime(vector<string> values);
    static float getSysIdleCpuTime(vector<string>values);
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
    break;
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

string ProcessParser::getProcUser(string pid) {
  string key("Uid:");
  // Once Uid: line is found in pid status, use this 0 based
  // index using space as delimiter to extract uid.
  int uidIndex = 1;
  
  ifstream fStatus;
  Util::getStream(Path::basePath() + pid + Path::statusPath(), fStatus);
  string line;
  string uid;
  while (getline(fStatus, line)) {
    if (line.find(key) != 0)
      continue;
    vector<string> lineSubStrs = Util::streamLineToStringVector(line);
    uid = lineSubStrs[uidIndex];
    break;
  }
  // Now search /etc/passwd result using uid extracted above
  // There will be a line "user:x:uid:blah blah blah"
  // The idea is to search for :x:uid and extract the user name just
  // left of that.
  string searchStr = ":x:" + uid;
  ifstream fPasswd;
  Util::getStream("/etc/passwd", fPasswd);
  string user;
  size_t pos;
  while (getline(fPasswd, line)) {
    pos = line.find(searchStr);
    if (pos == string::npos)
      continue;
    user = line.substr(0, pos);
    break;
  }
  return user;
}

vector<string> ProcessParser::getPidList() {
  DIR *procDir;
  if (!(procDir = opendir("/proc"))) {
    throw runtime_error(strerror(errno));
  }
  
  vector<string> pidList;
  while (dirent *de = readdir(procDir)) {
    if (all_of(de->d_name, de->d_name + strlen(de->d_name), [](char c) {
      return isdigit(c); })) {
      pidList.push_back(de->d_name);
    }
  }
  if (closedir(procDir)) {
    throw runtime_error(strerror(errno));
  }
  return pidList;
}

string ProcessParser::getCmd(string pid) {
  ifstream fCmd;
  Util::getStream(Path::basePath() + pid + "/" + Path::cmdPath(), fCmd);
  string result;
  getline(fCmd, result);
  return result;
}

vector<string> ProcessParser::getSysCpuPercent(string coreNumber) {
  string key = "cpu" + coreNumber;
  ifstream fStat;
  Util::getStream(Path::basePath() + Path::statPath(), fStat);
  vector<string> lineSubStrs;
  string line;
  while (getline(fStat, line)) {
    if (line.find(key) != 0)
      continue;
    lineSubStrs = Util::streamLineToStringVector(line);
    break;
  }
  return lineSubStrs;
}

float ProcessParser::getSysActiveCpuTime(vector<string> values) {
  return (stof(values[S_USER]) +
          stof(values[S_NICE]) +
          stof(values[S_SYSTEM]) +
          stof(values[S_IRQ]) +
          stof(values[S_SOFTIRQ]) +
          stof(values[S_STEAL]) +
          stof(values[S_GUEST]) +
          stof(values[S_GUEST_NICE]));
}

float ProcessParser::getSysIdleCpuTime(vector<string>values) {
  return (stof(values[S_IDLE]) + stof(values[S_IOWAIT]));
}

string ProcessParser::PrintCpuStats(std::vector<std::string> values1, std::vector<std::string>values2) {
  float activeTime = getSysActiveCpuTime(values2) - getSysActiveCpuTime(values1);
  float idleTime = getSysIdleCpuTime(values2) - getSysIdleCpuTime(values1);
  return to_string((activeTime / (activeTime + idleTime)) * 100);
}

