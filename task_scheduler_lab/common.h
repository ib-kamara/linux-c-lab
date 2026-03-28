#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <string>
#include <random>

using namespace std;

struct Process
{
    string id;
    int arrival_time;
    int burst_time;
    int priority;
    int remaining_time;
    int waiting_time;
    int turnaround_time;
    double vruntime;
    int deadline;
};

void calculateMetrics(vector<Process>& processes, int total_time)
{
    double avg_wait = 0;
    double avg_turn = 0;

    for (auto& p : processes)
    {
        avg_wait += p.waiting_time;
        avg_turn += p.turnaround_time;
    }

    avg_wait /= processes.size();
    avg_turn /= processes.size();

    double cpu_util = 0;
    if (total_time > 0)
    {
        cpu_util = (double) total_time / total_time * 100;
    }

    cout << "Avg Waiting Time: " << avg_wait << "\n";
    cout << "Avg Turnaround Time: " << avg_turn << "\n";
    cout << "CPU Utilization: " << cpu_util << "%\n";
}

void printGantt(const vector<pair<string, int>>& gantt)
{
    cout << "Gantt Chart:\n";
    for (auto& entry : gantt)
    {
        cout << entry.first << "(" << entry.second << ") ";
    }
    cout << "\n";
}

vector<Process> getProcesses()
{
    vector<Process> processes =
    {
        {"P1", 0, 8, 2, 8, 0, 0, 0.0, 0},
        {"P2", 1, 4, 1, 4, 0, 0, 0.0, 0},
        {"P3", 2, 9, 3, 9, 0, 0, 0.0, 0},
        {"P4", 3, 5, 4, 5, 0, 0, 0.0, 0}
    };

    return processes;
}

void printProcessTable(const vector<Process>& processes)
{
    cout << "\nProcess Times:\n";
    cout << "ID\tArrival\tBurst\tPriority\tWaiting\tTurnaround\n";

    for (const auto& p : processes)
    {
        cout << p.id << "\t"
             << p.arrival_time << "\t"
             << p.burst_time << "\t"
             << p.priority << "\t\t"
             << p.waiting_time << "\t"
             << p.turnaround_time << "\n";
    }
}

#endif