#include "common.h"

int main()
{
    vector<Process> processes = getProcesses();
    vector<pair<string, int>> gantt;
    int current_time = 0;
    int completed = 0;
    int n = processes.size();
    vector<bool> done(n, false);

    while (completed < n)
    {
        int shortest_index = -1;

        for (int i = 0; i < n; i++)
        {
            if (!done[i] && processes[i].arrival_time <= current_time)
            {
                if (shortest_index == -1 || processes[i].burst_time < processes[shortest_index].burst_time)
                {
                    shortest_index = i;
                }
            }
        }

        if (shortest_index == -1)
        {
            current_time++;
        }
        else
        {
            processes[shortest_index].waiting_time = current_time - processes[shortest_index].arrival_time;
            gantt.push_back({processes[shortest_index].id, processes[shortest_index].burst_time});
            current_time += processes[shortest_index].burst_time;
            processes[shortest_index].turnaround_time = current_time - processes[shortest_index].arrival_time;
            done[shortest_index] = true;
            completed++;
        }
    }

    calculateMetrics(processes, current_time);
    printProcessTable(processes);
    printGantt(gantt);

    return 0;
}