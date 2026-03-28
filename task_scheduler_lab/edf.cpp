#include "common.h"

int main()
{
    vector<Process> processes = getProcesses();

    for (auto& p : processes)
    {
        p.remaining_time = p.burst_time;
        p.deadline = p.arrival_time + p.burst_time * 2;
    }

    vector<pair<string, int>> gantt;
    int current_time = 0;
    int completed = 0;
    int n = processes.size();

    while (completed < n)
    {
        int chosen = -1;

        for (int i = 0; i < n; i++)
        {
            if (processes[i].arrival_time <= current_time && processes[i].remaining_time > 0)
            {
                if (chosen == -1 || processes[i].deadline < processes[chosen].deadline)
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
            gantt[gantt.size() - 1].second++;
        }
        else
        {
            gantt.push_back({processes[chosen].id, 1});
        }

        processes[chosen].remaining_time--;
        current_time++;

        if (processes[chosen].remaining_time == 0)
        {
            completed++;
            processes[chosen].turnaround_time = current_time - processes[chosen].arrival_time;
            processes[chosen].waiting_time =
                processes[chosen].turnaround_time - processes[chosen].burst_time;
        }
    }

    cout << "Deadlines:\n";
    for (auto& p : processes)
    {
        cout << p.id << ": " << p.deadline << "\n";
    }

    calculateMetrics(processes, current_time);
    printProcessTable(processes);
    printGantt(gantt);

    return 0;
}