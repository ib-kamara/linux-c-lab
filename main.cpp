#include <iostream>
#include <fstream>
#include <sstream>
#include "VirtualMemory.h"
#include "Clock.h"

int main()
{
    VirtualMemory memory(8);
    Clock clock(16);

    std::ifstream inputFile("input/trace.txt");

    if (!inputFile)
    {
        std::cout << "Error: Could not open input/trace.txt" << std::endl;
        return 1;
    }

    int pid;
    std::string addressText;

    while (inputFile >> pid >> addressText)
    {
        int virtualAddress = std::stoi(addressText, nullptr, 16);
        int pageNumber = virtualAddress >> 12;
        int physicalAddress;

        std::cout << std::endl;
        std::cout << "PID: " << pid << std::endl;
        std::cout << "Virtual Address: " << addressText << std::endl;
        std::cout << "Page Number: " << pageNumber << std::endl;

        bool found = memory.translateAddress(pid, virtualAddress, physicalAddress);

        if (!found)
        {
            int evictedPage;
            int frameNumber = clock.evictAndReplace(pid, pageNumber, evictedPage);

            if (evictedPage != -1)
            {
                std::cout << "Evicted Page: " << evictedPage << std::endl;
            }

            std::cout << "Loaded into Frame: " << frameNumber << std::endl;

            memory.loadPage(pid, pageNumber, frameNumber);
            memory.translateAddress(pid, virtualAddress, physicalAddress);
        }

        std::cout << "Physical Address: " << physicalAddress << std::endl;
    }

    memory.printStats();
    clock.printFrames();

    return 0;
}