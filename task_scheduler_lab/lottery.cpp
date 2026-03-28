#include "common.h"

int main()
{
    vector<Process> processes = getProcesses();

    for (auto& p : processes)
    {
        p.remaining_time = p.burst_time;
    }

    random_device rd;
    mt19937 gen(rd());

    vector<pair<string, int>> gantt;
    int current_time = 0;
    int completed = 0;
    int n = processes.size();

    while (completed < n)
    {
        vector<int> ready;
        vector<int> tickets;

        for (int i = 0; i < n; i++)
        {
            if (processes[i].arrival_time <= current_time && processes[i].remaining_time > 0)
            {
                ready.push_back(i);

                int ticket_count = 10 / processes[i].priority;
                if (ticket_count < 1)
                {
                    ticket_count = 1;
                }

                for (int j = 0; j < ticket_count; j++)
                {
                    tickets.push_back(i);
                }
            }
        }

        if (tickets.empty())
        {
            current_time++;
            continue;
        }

        uniform_int_distribution<> dist(0, tickets.size() - 1);
        int winner = tickets[dist(gen)];

        if (!gantt.empty() && gantt[gantt.size() - 1].first == processes[winner].id)
        {
            gantt[gantt.size() - 1].second++;
        }
        else
        {
            gantt.push_back({processes[winner].id, 1});
        }

        processes[winner].remaining_time--;
        current_time++;

        if (processes[winner].remaining_time == 0)
        {
            completed++;
            processes[winner].turnaround_time = current_time - processes[winner].arrival_time;
            processes[winner].waiting_time =
                processes[winner].turnaround_time - processes[winner].burst_time;
        }
    }

    calculateMetrics(processes, current_time);
    printProcessTable(processes);
    printGantt(gantt);

    return 0;
}