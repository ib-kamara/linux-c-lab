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
        int highest_priority_index = -1;

        for (int i = 0; i < n; i++)
        {
            if (!done[i] && processes[i].arrival_time <= current_time)
            {
                if (highest_priority_index == -1 || processes[i].priority < processes[highest_priority_index].priority)
                {
                    highest_priority_index = i;
                }
            }
        }

        if (highest_priority_index == -1)
        {
            current_time++;
        }
        else
        {
            processes[highest_priority_index].waiting_time =
                current_time - processes[highest_priority_index].arrival_time;

            gantt.push_back({processes[highest_priority_index].id, processes[highest_priority_index].burst_time});

            current_time += processes[highest_priority_index].burst_time;

            processes[highest_priority_index].turnaround_time =
                current_time - processes[highest_priority_index].arrival_time;

            done[highest_priority_index] = true;
            completed++;
        }
    }

    calculateMetrics(processes, current_time);
    printProcessTable(processes);
    printGantt(gantt);

    return 0;
}