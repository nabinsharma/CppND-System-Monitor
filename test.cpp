#include "ProcessParser.h"

int main(void) {
  std::string pid("22");
  ProcessParser pp;
  
  long int sysUpTimeSec = pp.getSysUpTime();
  cout << "System uptime is " << sysUpTimeSec << " S \n";
  
  std::string memUsage = pp.getVmSize(pid);
  cout << "PID " << pid << " is using " << memUsage << " MB \n";
  
  std::string upTimeSec = pp.getProcUpTime(pid);
  cout << "PID " << pid << " uptime is " << upTimeSec << " S \n";
  
  std::string cpuUsage = pp.getCpuPercent(pid);
  cout << "PID " << pid << " CPU usage is " << cpuUsage << " % \n";
}
