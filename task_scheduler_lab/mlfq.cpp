#include "common.h"

int main()
{
    vector<Process> processes = getProcesses();

    for (auto& p : processes)
    {
        p.remaining_time = p.burst_time;
    }

    vector<queue<int>> queues(3);
    vector<pair<string, int>> gantt;
    vector<int> level(processes.size(), 0);
    vector<bool> added(processes.size(), false);

    int quantums[3] = {2, 4, 8};
    int current_time = 0;
    int completed = 0;
    int n = processes.size();

    while (completed < n)
    {
        for (int i = 0; i < n; i++)
        {
            if (!added[i] && processes[i].arrival_time <= current_time)
            {
                queues[0].push(i);
                added[i] = true;
            }
        }

        int chosen_queue = -1;

        for (int i = 0; i < 3; i++)
        {
            if (!queues[i].empty())
            {
                chosen_queue = i;
                break;
            }
        }

        if (chosen_queue == -1)
        {
            current_time++;
            continue;
        }

        int index = queues[chosen_queue].front();
        queues[chosen_queue].pop();

        int run_time;
        if (processes[index].remaining_time < quantums[chosen_queue])
        {
            run_time = processes[index].remaining_time;
        }
        else
        {
            run_time = quantums[chosen_queue];
        }

        gantt.push_back({processes[index].id, run_time});
        processes[index].remaining_time -= run_time;
        current_time += run_time;

        for (int i = 0; i < n; i++)
        {
            if (!added[i] && processes[i].arrival_time <= current_time)
            {
                queues[0].push(i);
                added[i] = true;
            }
        }

        if (processes[index].remaining_time > 0)
        {
            if (chosen_queue < 2)
            {
                level[index] = chosen_queue + 1;
                queues[chosen_queue + 1].push(index);
            }
            else
            {
                queues[2].push(index);
            }
        }
        else
        {
            completed++;
            processes[index].turnaround_time = current_time - processes[index].arrival_time;
            processes[index].waiting_time =
                processes[index].turnaround_time - processes[index].burst_time;
        }
    }

    calculateMetrics(processes, current_time);
    printProcessTable(processes);
    printGantt(gantt);

    return 0;
}