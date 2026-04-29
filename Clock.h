#ifndef CLOCK_H
#define CLOCK_H

#include <iostream>
#include <vector>

struct FrameEntry
{
    int pid;
    int pageNumber;
    bool referenced;
};

class Clock
{
private:
    std::vector<FrameEntry> frames;
    int clockHand;
    int frameCount;

public:
    Clock(int frameCount);
    int evictAndReplace(int pid, int pageNumber, int &evictedPage);
    void printFrames() const;
};

#endif