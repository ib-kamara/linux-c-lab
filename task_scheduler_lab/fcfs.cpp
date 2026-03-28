#include "common.h"

int main()
{
    vector<Process> processes = getProcesses();

    sort(processes.begin(), processes.end(),
         [](const Process& a, const Process& b)
         {
             return a.arrival_time < b.arrival_time;
         });

    vector<pair<string, int>> gantt;
    int current_time = 0;

    for (auto& p : processes)
    {
        if (current_time < p.arrival_time)
        {
            current_time = p.arrival_time;
        }

        p.waiting_time = current_time - p.arrival_time;
        gantt.push_back({p.id, p.burst_time});
        current_time += p.burst_time;
        p.turnaround_time = current_time - p.arrival_time;
    }

    calculateMetrics(processes, current_time);
    printProcessTable(processes);
    printGantt(gantt);

    return 0;
}