#include "common.h"

int main()
{
    vector<Process> processes = getProcesses();

    for (auto& p : processes)
    {
        p.remaining_time = p.burst_time;
    }

    vector<pair<string, int>> gantt;
    int current_time = 0;
    int completed = 0;
    int n = processes.size();
    string last_process = "";

    while (completed < n)
    {
        int shortest_index = -1;

        for (int i = 0; i < n; i++)
        {
            if (processes[i].arrival_time <= current_time && processes[i].remaining_time > 0)
            {
                if (shortest_index == -1 || processes[i].remaining_time < processes[shortest_index].remaining_time)
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
            if (last_process != processes[shortest_index].id)
            {
                gantt.push_back({processes[shortest_index].id, 1});
            }
            else
            {
                gantt[gantt.size() - 1].second++;
            }

            last_process = processes[shortest_index].id;
            processes[shortest_index].remaining_time--;
            current_time++;

            if (processes[shortest_index].remaining_time == 0)
            {
                completed++;
                processes[shortest_index].turnaround_time = current_time - processes[shortest_index].arrival_time;
                processes[shortest_index].waiting_time =
                    processes[shortest_index].turnaround_time - processes[shortest_index].burst_time;
            }
        }
    }

    calculateMetrics(processes, current_time);
    printProcessTable(processes);
    printGantt(gantt);

    return 0;
}