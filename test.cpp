#include <time.h>
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
  
  std::string user = pp.getProcUser(pid);
  cout << "PID " << pid << " user is " << user << "\n";
  
  std::vector<std::string> pidList = pp.getPidList();
  cout << "PID List: ";
  for (std::vector<std::string>::iterator it = pidList.begin(); it != pidList.end(); it++) {
    cout << *it << ", ";
  }
  cout << "\n";
  
  cout << "PID " << pid << " cmdline is " << pp.getCmd(pid) << "\n";
  
  cout << "Number of CPU cores is " << pp.getNumberOfCores() << "\n";
  
  std::vector<std::string> cpuPercent = pp.getSysCpuPercent();
  cout << "cpuPercent info is ";
  for (std::vector<std::string>::iterator it = cpuPercent.begin(); it != cpuPercent.end(); it++) {
    cout << *it << ", ";
  }
  cout << "\n";  
  
  vector<string> values1 = pp.getSysCpuPercent();
  struct timespec sleep;
  sleep.tv_sec = 2;
  sleep.tv_nsec = 0;
  if(nanosleep(&sleep , NULL)) {
    throw runtime_error(strerror(errno));
  }
  vector<string> values2 = pp.getSysCpuPercent();
  cout << "CPU stat is " << pp.PrintCpuStats(values2, values1) << "% \n";
  
  cout << "RAM usage is " << pp.getSysRamPercent() << "% \n";
  
  cout << "Linux version is " << pp.getSysKernelVersion() << " \n";
  
  cout << "OS name is " << pp.getOSName() << " \n";
  
  cout << "Number of total threads is " << pp.getTotalThreads() << " \n";
  
  int processes = pp.getTotalNumberOfProcesses();
  int runningProcesses = pp.getNumberOfRunningProcesses();
  cout << "Number of running/total processes is " << runningProcesses << " / " << processes << "\n";
  
  bool found = pp.isPidExisting(pid);
  if (found)
    cout << "PID " + pid + " exists" << "\n";
  else
    cout << "PID " + pid + " does not exist" << "\n";
}
