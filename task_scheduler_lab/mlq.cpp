#include "common.h"

int main()
{
    vector<Process> processes = getProcesses();

    for (auto& p : processes)
    {
        p.remaining_time = p.burst_time;
    }

    queue<int> high;
    queue<int> low;
    vector<pair<string, int>> gantt;
    int current_time = 0;
    int quantum = 4;
    int completed = 0;
    int n = processes.size();
    vector<bool> added(n, false);

    while (completed < n)
    {
        for (int i = 0; i < n; i++)
        {
            if (!added[i] && processes[i].arrival_time <= current_time)
            {
                if (processes[i].priority < 3)
                {
                    high.push(i);
                }
                else
                {
                    low.push(i);
                }
                added[i] = true;
            }
        }

        if (!high.empty())
        {
            int index = high.front();
            high.pop();

            int run_time;
            if (processes[index].remaining_time < quantum)
            {
                run_time = processes[index].remaining_time;
            }
            else
            {
                run_time = quantum;
            }

            gantt.push_back({processes[index].id, run_time});
            processes[index].remaining_time -= run_time;
            current_time += run_time;

            for (int i = 0; i < n; i++)
            {
                if (!added[i] && processes[i].arrival_time <= current_time)
                {
                    if (processes[i].priority < 3)
                    {
                        high.push(i);
                    }
                    else
                    {
                        low.push(i);
                    }
                    added[i] = true;
                }
            }

            if (processes[index].remaining_time > 0)
            {
                high.push(index);
            }
            else
            {
                completed++;
                processes[index].turnaround_time = current_time - processes[index].arrival_time;
                processes[index].waiting_time =
                    processes[index].turnaround_time - processes[index].burst_time;
            }
        }
        else if (!low.empty())
        {
            int index = low.front();
            low.pop();

            gantt.push_back({processes[index].id, processes[index].remaining_time});
            current_time += processes[index].remaining_time;
            processes[index].remaining_time = 0;
            completed++;

            processes[index].turnaround_time = current_time - processes[index].arrival_time;
            processes[index].waiting_time =
                processes[index].turnaround_time - processes[index].burst_time;
        }
        else
        {
            current_time++;
        }
    }

    calculateMetrics(processes, current_time);
    printProcessTable(processes);
    printGantt(gantt);

    return 0;
}