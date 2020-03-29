#include "processor.h"
#include "linux_parser.h"
#include <string>
#include <iostream>

// TODO: Return the aggregate CPU utilization
float Processor::Utilization() {
    cpuStringData = LinuxParser::CpuUtilization();
    cpuFloatData = {};

    for (const auto & i : cpuStringData) {
        cpuFloatData.push_back(std::stof(i));
    }

    idle = cpuFloatData[3] + cpuFloatData[4];

    active = cpuFloatData[0] + cpuFloatData[1] + cpuFloatData[2] +
             cpuFloatData[5] + cpuFloatData[6] + cpuFloatData[7];

    total = idle + active;

    dTotal = total - prevTotal;
    dActive = active - prevActive;
    prevTotal = total;
    prevActive = active;
    return dActive / dTotal;
}