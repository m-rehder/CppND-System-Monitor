#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <sstream>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key, value;
  std::ifstream fileStream(kOSPath);
  if (fileStream.is_open()) {
    while (std::getline(fileStream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream lineStream(line);
      while (lineStream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream lineStream(line);
    lineStream >> os >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// TODO: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
    string line;
    string key;
    long memValue;
    long memTotal = 0;
    long memFree = 0;
    long memAvailable = 0;
    long memBuffers;
    string unit;
    std::ifstream fileStream(kProcDirectory + kMeminfoFilename);

    if (fileStream.is_open()) {
        int countToRead = 0;
        while (countToRead < 4 && std::getline(fileStream, line)) {
            std::istringstream linestream(line);
            linestream >> key >> memValue >> unit ;
            if (key == "MemTotal:") {
                memTotal = memValue;
                countToRead++;
                continue;
            }
            if (key == "MemFree:") {
                memFree = memValue;
                countToRead++;
                continue;
            }
            if (key == "MemAvailable:") {
                memAvailable = memValue;
                countToRead++;
                continue;
            }
            if (key == "Buffers:") {
                memBuffers = memValue;
                countToRead++;
            }

        }
    }
    float memFreeGB = memFree/(1024*1024);
    float memTotalGB = memTotal/(1024*1024);
    return (memTotalGB - memFreeGB)/memTotalGB;
}

// TODO: Read and return the system uptime
long LinuxParser::UpTime() {
    std::ifstream fileStream(kProcDirectory + kUptimeFilename);
    long upTime = 0;
    long idleTime;
    if (fileStream.is_open()) {
        std::string line;
        std::getline(fileStream, line);
        std::istringstream lineStream(line);
        lineStream >> upTime >> idleTime;
    }
    return upTime;
}

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
    std::ifstream fileStream(kProcDirectory + kStatFilename);
    if (fileStream.is_open()) {
        std::string line;
        std::getline(fileStream, line);
        std::istringstream lineStream(line);
        std::string cpu;
        long user;
        long nice;
        long system;
        long idle;
        long ioWait;
        long irq;
        long softIrq;
        long steal;
        long guess;
        long guessNice;
        lineStream >> cpu >> user >> nice >> system >> idle >> ioWait >> irq >> softIrq >> steal >> guess >> guessNice;
        long totalUserTime = user - guess;
        long totalNiceTime = nice - guessNice;
        long totalIdleTime = idle + ioWait;
        long totalSystem = system + irq + softIrq;
        long totalVirtualTime = guess + guessNice;
        return totalUserTime + totalNiceTime + totalIdleTime + totalSystem + totalVirtualTime;
    }
    return 0;
}

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid) {
    std::stringstream filename;
    filename << kProcDirectory << "/" << pid << "/" << kStatFilename;
    std::ifstream fileStream(filename.str());
    if (fileStream.is_open()) {
        string line;
        std::getline(fileStream, line);
        std::istringstream lineStream(line);
        std::string ignore;
        long utime;
        long stime;
        long cutime;
        long cstime;
        long starttime;
        for(int i = 0; i < 13; i++) lineStream >> ignore;
        lineStream >> utime >> stime >> cutime >> cstime ;
        for(int i = 0; i < 4; i++) lineStream >> ignore;
        lineStream >> starttime;
        return utime + stime + cutime + cstime + starttime;
    }
}

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
    return Jiffies() - IdleJiffies();
}

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
    string line;
    string key;
    long idleJiffies = 0;
    std::ifstream fileStream(kProcDirectory + kStatFilename);
    if (fileStream.is_open()) {
        std::getline(fileStream, line);
        std::stringstream lineStream(line);
        int value;
        for (int i = 0; i < 6; ++i) {
            if (i == 0) {
                lineStream >> key;
            }
            else if (i > 3) {
                lineStream >> value;
                idleJiffies += value;
            }
            else {
                lineStream >> value;
            }
        }
    }
    return idleJiffies;
}

// TODO: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
    string line;
    string value;
    vector<string> cpuUtil;
    std::ifstream fileStream(kProcDirectory + kStatFilename);
    if (fileStream.is_open()) {
        std::getline(fileStream, line);
        std::stringstream lineStream(line);
        while (lineStream >> value) {
            if (value != "cpu") {
                cpuUtil.push_back(value);
            }
        }
    }
    return cpuUtil;
}

// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() {
    string line;
    string key, value;
    std::ifstream fileStream(kProcDirectory + kStatFilename);
    if (fileStream.is_open()) {
        while (std::getline(fileStream, line)) {
            std::istringstream lineStream(line);
            while (lineStream >> key >> value) {
                if (key == "processes") {
                    return std::stoi(value);
                }
            }
        }
    }
    return 0;
}

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
    string line;
    string key, value;
    std::ifstream fileStream(kProcDirectory + kStatFilename);
    if (fileStream.is_open()) {
        while (std::getline(fileStream, line)) {
            std::istringstream lineStream(line);
            while (lineStream >> key >> value) {
                if (key == "procs_running") {
                    return std::stoi(value);
                }
            }
        }
    }
    return 0;
}

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid) {
    string line;
    string command;
    std::ifstream fileStream(kProcDirectory + std::to_string(pid) +
                             kCmdlineFilename);
    if (fileStream.is_open()) {
        while (std::getline(fileStream, line)) {
            std::istringstream lineStream(line);
            std::replace(line.begin(), line.end(), ' ', '+');
            while (lineStream >> command) {
                std::replace(line.begin(), line.end(), '+', ' ');
            }
            return command;
        }
    }
    return string();
}

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid) {
    string line;
    string key, value;
    int vmSize{0};
    std::ifstream fileStream(kProcDirectory + std::to_string(pid) +
                             kStatusFilename);
    if (fileStream.is_open()) {
        while (std::getline(fileStream, line)) {
            std::istringstream lineStream(line);
            while (lineStream >> key >> value) {
                if (key == "VmSize:") {
                    vmSize = std::stoi(value);
                    vmSize = vmSize / 1000;
                    return std::to_string(vmSize);
                }
            }
        }
    }
    return string();
}

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid) {
    string line;
    string key, value;
    std::ifstream fileStream(kProcDirectory + std::to_string(pid) +
                             kStatusFilename);
    if (fileStream.is_open()) {
        while (std::getline(fileStream, line)) {
            std::istringstream lineStream(line);
            while (lineStream >> key >> value) {
                if (key == "Uid:") {
                    return value;
                }
            }
        }
    }
    return string();
}

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid) {
    string line;
    string user, x, uid;
    string uidTarget = Uid(pid);
    std::ifstream fileStream(kPasswordPath);
    if (fileStream.is_open()) {
        while (std::getline(fileStream, line)) {
            std::replace(line.begin(), line.end(), ':', ' ');
            std::istringstream lineStream(line);
            while (lineStream >> user >> x >> uid) {
                if (uid == uidTarget) {
                    return user;
                }
            }
        }
    }
    return string();
}

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid) {
    string line, value;
    long int starttime{0}, uptime{0};
    vector<string> statData = {};
    std::ifstream fileStream(kProcDirectory + std::to_string(pid) +
                             kStatFilename);
    if (fileStream.is_open()) {
        std::getline(fileStream, line);
        std::istringstream lineStream(line);
        while (lineStream >> value) {
            statData.push_back(value);
        }
    }
    starttime = std::stol(statData[21]) / sysconf(_SC_CLK_TCK);
    uptime = LinuxParser::UpTime() - starttime;
    return uptime;
}