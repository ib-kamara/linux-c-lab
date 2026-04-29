#include "VirtualMemory.h"

VirtualMemory::VirtualMemory(int size)
{
    tlbSize = size;
    currentTime = 0;
    tlbHits = 0;
    tlbMisses = 0;
}

bool VirtualMemory::lookupTLB(int pid, int pageNumber, int &frameNumber)
{
    currentTime++;

    for (int i = 0; i < tlb.size(); i++)
    {
        if (tlb[i].pid == pid && tlb[i].pageNumber == pageNumber)
        {
            frameNumber = tlb[i].frameNumber;
            tlb[i].lastUsed = currentTime;
            tlbHits++;
            std::cout << "TLB Hit" << std::endl;
            return true;
        }
    }

    tlbMisses++;
    std::cout << "TLB Miss" << std::endl;
    return false;
}

bool VirtualMemory::translateAddress(int pid, int virtualAddress, int &physicalAddress)
{
    int outerPage = (virtualAddress >> 22) & 0x3FF;
    int innerPage = (virtualAddress >> 12) & 0x3FF;
    int offset = virtualAddress & 0xFFF;

    int pageNumber = virtualAddress >> 12;
    int frameNumber;

    if (lookupTLB(pid, pageNumber, frameNumber))
    {
        physicalAddress = frameNumber * 4096 + offset;
        return true;
    }

    std::pair<int, int> outerKey(pid, outerPage);
    std::pair<int, int> innerKey(pid, innerPage);

    if (outerPageTable.find(outerKey) == outerPageTable.end())
    {
        std::cout << "Page Fault" << std::endl;
        return false;
    }

    if (innerPageTable.find(innerKey) == innerPageTable.end())
    {
        std::cout << "Page Fault" << std::endl;
        return false;
    }

    frameNumber = innerPageTable[innerKey];
    insertTLB(pid, pageNumber, frameNumber);

    physicalAddress = frameNumber * 4096 + offset;
    return true;
}

void VirtualMemory::insertTLB(int pid, int pageNumber, int frameNumber)
{
    currentTime++;

    TLBEntry newEntry;
    newEntry.pid = pid;
    newEntry.pageNumber = pageNumber;
    newEntry.frameNumber = frameNumber;
    newEntry.lastUsed = currentTime;

    if (tlb.size() < tlbSize)
    {
        tlb.push_back(newEntry);
    }
    else
    {
        int leastUsedIndex = 0;

        for (int i = 1; i < tlb.size(); i++)
        {
            if (tlb[i].lastUsed < tlb[leastUsedIndex].lastUsed)
            {
                leastUsedIndex = i;
            }
        }

        tlb[leastUsedIndex] = newEntry;
    }
}

void VirtualMemory::loadPage(int pid, int pageNumber, int frameNumber)
{
    int outerPage = (pageNumber >> 10) & 0x3FF;
    int innerPage = pageNumber & 0x3FF;

    outerPageTable[std::make_pair(pid, outerPage)] = 1;
    innerPageTable[std::make_pair(pid, innerPage)] = frameNumber;

    insertTLB(pid, pageNumber, frameNumber);
}

void VirtualMemory::printStats() const
{
    int total = tlbHits + tlbMisses;

    std::cout << std::endl;
    std::cout << "Virtual Memory Statistics" << std::endl;
    std::cout << "TLB Hits: " << tlbHits << std::endl;
    std::cout << "TLB Misses: " << tlbMisses << std::endl;

    if (total > 0)
    {
        double hitRate = (double)tlbHits / total * 100;
        std::cout << "TLB Hit Rate: " << hitRate << "%" << std::endl;
    }
}