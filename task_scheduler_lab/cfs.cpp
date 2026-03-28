#include "common.h"

int main()
{
    vector<Process> processes = getProcesses();

    for (auto& p : processes)
    {
        p.remaining_time = p.burst_time;
        p.vruntime = 0.0;
    }

    vector<pair<string, int>> gantt;
    int current_time = 0;
    int completed = 0;
    int n = processes.size();
    int slice = 1;

    while (completed < n)
    {
        int chosen = -1;

        for (int i = 0; i < n; i++)
        {
            if (processes[i].arrival_time <= current_time && processes[i].remaining_time > 0)
            {
                if (chosen == -1 || processes[i].vruntime < processes[chosen].vruntime)
                {
                    chosen = i;
                }
            }
        }

        if (chosen == -1)
        {
            current_time++;
            continue;
        }

        if (!gantt.empty() && gantt[gantt.size() - 1].first == processes[chosen].id)
        {
            gantt[gantt.size() - 1].second += slice;
        }
        else
        {
            gantt.push_back({processes[chosen].id, slice});
        }

        processes[chosen].remaining_time -= slice;
        current_time += slice;

        double weight = 1.0 / processes[chosen].priority;
        processes[chosen].vruntime += slice / weight;

        if (processes[chosen].remaining_time == 0)
        {
            completed++;
            processes[chosen].turnaround_time = current_time - processes[chosen].arrival_time;
            processes[chosen].waiting_time =
                processes[chosen].turnaround_time - processes[chosen].burst_time;
        }
    }

    calculateMetrics(processes, current_time);
    printProcessTable(processes);

    cout << "\nVRuntime Values:\n";
    for (auto& p : processes)
    {
        cout << p.id << ": " << p.vruntime << "\n";
    }

    printGantt(gantt);

    return 0;
}