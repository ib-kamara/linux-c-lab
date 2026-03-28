#include "common.h"

int main()
{
    vector<Process> processes = getProcesses();

    for (auto& p : processes)
    {
        p.remaining_time = p.burst_time;
    }

    queue<int> ready_queue;
    vector<pair<string, int>> gantt;
    int current_time = 0;
    int quantum = 4;
    int completed = 0;
    int n = processes.size();
    vector<bool> in_queue(n, false);

    while (completed < n)
    {
        for (int i = 0; i < n; i++)
        {
            if (!in_queue[i] && processes[i].arrival_time <= current_time && processes[i].remaining_time > 0)
            {
                ready_queue.push(i);
                in_queue[i] = true;
            }
        }

        if (ready_queue.empty())
        {
            current_time++;
            continue;
        }

        int index = ready_queue.front();
        ready_queue.pop();

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
            if (!in_queue[i] && processes[i].arrival_time <= current_time && processes[i].remaining_time > 0)
            {
                ready_queue.push(i);
                in_queue[i] = true;
            }
        }

        if (processes[index].remaining_time > 0)
        {
            ready_queue.push(index);
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