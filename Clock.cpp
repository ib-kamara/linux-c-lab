#include "Clock.h"

Clock::Clock(int count)
{
    frameCount = count;
    clockHand = 0;

    for (int i = 0; i < frameCount; i++)
    {
        FrameEntry frame;
        frame.pid = -1;
        frame.pageNumber = -1;
        frame.referenced = false;
        frames.push_back(frame);
    }
}

int Clock::evictAndReplace(int pid, int pageNumber, int &evictedPage)
{
    while (true)
    {
        if (frames[clockHand].pid == -1)
        {
            frames[clockHand].pid = pid;
            frames[clockHand].pageNumber = pageNumber;
            frames[clockHand].referenced = true;
            evictedPage = -1;

            int usedFrame = clockHand;
            clockHand = (clockHand + 1) % frameCount;
            return usedFrame;
        }

        if (frames[clockHand].referenced == false)
        {
            evictedPage = frames[clockHand].pageNumber;

            frames[clockHand].pid = pid;
            frames[clockHand].pageNumber = pageNumber;
            frames[clockHand].referenced = true;

            int usedFrame = clockHand;
            clockHand = (clockHand + 1) % frameCount;
            return usedFrame;
        }

        frames[clockHand].referenced = false;
        clockHand = (clockHand + 1) % frameCount;
    }
}

void Clock::printFrames() const
{
    std::cout << std::endl;
    std::cout << "Current Physical Frames" << std::endl;

    for (int i = 0; i < frames.size(); i++)
    {
        std::cout << "Frame " << i << ": ";

        if (frames[i].pid == -1)
        {
            std::cout << "Empty" << std::endl;
        }
        else
        {
            std::cout << "PID " << frames[i].pid
                      << ", Page " << frames[i].pageNumber
                      << ", Referenced " << frames[i].referenced
                      << std::endl;
        }
    }
}