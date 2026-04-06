#include <iostream>
#include <vector>
#include <map>
#include <queue>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <string>
#include <random>
#include <algorithm>

using namespace std;

enum Protection { READ_ONLY, READ_WRITE };

struct Page {
    int frame_number = -1;
    bool present = false;
    Protection protection = READ_WRITE;
    int last_access = 0;
};

struct Segment {
    int base_address;
    int limit;
    Protection protection;
};

class PageTable {
public:
    vector<Page> pages;
    int page_size;

    PageTable() {
        page_size = 1000;
    }

    PageTable(int numPages, int pageSize) : page_size(pageSize) {
        pages.resize(numPages);
        for (auto &p : pages) {
            p.present = rand() % 2;
            if (p.present) {
                p.frame_number = rand() % 100;
            }
            p.protection = (rand() % 2) ? READ_WRITE : READ_ONLY;
            p.last_access = 0;
        }
    }

    int getFrameNumber(int pageNum, int time, Protection accessType) {
        if (pageNum < 0 || pageNum >= pages.size()) {
            cout << "Page Fault: Invalid page number " << pageNum << "\n";
            return -1;
        }

        if (!pages[pageNum].present) {
            cout << "Page Fault: Page " << pageNum << " not in memory\n";
            return -1;
        }

        if (accessType == READ_WRITE && pages[pageNum].protection == READ_ONLY) {
            cout << "Protection Violation: Cannot write to read-only page\n";
            return -1;
        }

        pages[pageNum].last_access = time;
        return pages[pageNum].frame_number;
    }

    void setFrame(int pageNum, int frame, Protection prot) {
        if (pageNum >= 0 && pageNum < pages.size()) {
            pages[pageNum].frame_number = frame;
            pages[pageNum].present = true;
            pages[pageNum].protection = prot;
        }
    }
};

class PhysicalMemory {
public:
    int num_frames;
    vector<bool> free_frames;
    queue<int> fifo_queue;
    int time = 0;

    PhysicalMemory(int frames) : num_frames(frames) {
        free_frames.resize(frames, true);
    }

    int allocateFrame() {
        for (int i = 0; i < num_frames; ++i) {
            if (free_frames[i]) {
                free_frames[i] = false;
                fifo_queue.push(i);
                return i;
            }
        }

        int frame = fifo_queue.front();
        fifo_queue.pop();
        fifo_queue.push(frame);
        return frame;
    }

    void freeFrame(int frame) {
        if (frame >= 0 && frame < num_frames) {
            free_frames[frame] = true;
        }
    }

    double utilization() const {
        int used = count(free_frames.begin(), free_frames.end(), false);
        return (double)used / num_frames * 100.0;
    }
};

class SegmentTable {
public:
    vector<Segment> segments;
    map<int, PageTable> pageTables;
    PhysicalMemory physMem;

    SegmentTable(int numFrames) : physMem(numFrames) {}

    void addSegment(int id, int base, int limit, Protection prot, int pageSize) {
        Segment s;
        s.base_address = base;
        s.limit = limit;
        s.protection = prot;
        segments.push_back(s);

        pageTables[id] = PageTable(limit, pageSize);
    }

    int translateAddress(int segNum, int pageNum, int offset, Protection accessType, int &latency) {
        physMem.time++;
        latency = 1 + rand() % 5;

        if (segNum < 0 || segNum >= segments.size()) {
            cout << "Segmentation Fault: Invalid segment " << segNum << "\n";
            return -1;
        }

        Segment segment = segments[segNum];

        if (pageNum < 0 || pageNum >= segment.limit) {
            cout << "Page Fault: Page " << pageNum << " exceeds limit " << segment.limit << "\n";
            return -1;
        }

        if (offset < 0 || offset >= pageTables[segNum].page_size) {
            cout << "Offset Fault: Offset " << offset << " exceeds page size\n";
            return -1;
        }

        if (accessType == READ_WRITE && segment.protection == READ_ONLY) {
            cout << "Protection Violation: Cannot write to read-only segment\n";
            return -1;
        }

        int frame = pageTables[segNum].getFrameNumber(pageNum, physMem.time, accessType);

        if (frame == -1) {
            frame = physMem.allocateFrame();
            pageTables[segNum].setFrame(pageNum, frame, accessType);
        }

        return segment.base_address + frame * pageTables[segNum].page_size + offset;
    }

    void printMemoryMap() {
        cout << "\nMemory Map:\n";
        for (size_t i = 0; i < segments.size(); ++i) {
            cout << "Segment " << i
                 << ": Base=" << segments[i].base_address
                 << ", Limit=" << segments[i].limit
                 << ", Protection=" << (segments[i].protection == READ_ONLY ? "RO" : "RW")
                 << "\n";

            for (size_t j = 0; j < pageTables[i].pages.size(); ++j) {
                Page p = pageTables[i].pages[j];
                cout << "  Page " << j
                     << ": Frame=" << p.frame_number
                     << ", Present=" << p.present
                     << ", Protection=" << (p.protection == READ_ONLY ? "RO" : "RW")
                     << ", LastAccess=" << p.last_access
                     << "\n";
            }
        }

        cout << "Physical Memory Utilization: " << physMem.utilization() << "%\n";
    }
};

void generateRandomAddresses(SegmentTable &st, int num, double validRatio, const string &logFile) {
    ofstream log(logFile);
    srand(time(0));

    int faults = 0;

    for (int i = 0; i < num; ++i) {
        int segNum;
        int pageNum;
        int offset;

        if (((double)rand() / RAND_MAX) < validRatio) {
            segNum = rand() % st.segments.size();
            pageNum = rand() % st.segments[segNum].limit;
            offset = rand() % st.pageTables[segNum].page_size;
        } else {
            segNum = rand() % (st.segments.size() + 2);
            pageNum = rand() % 10;
            offset = rand() % (st.pageTables[0].page_size + 100);
        }

        Protection access = (rand() % 2) ? READ_WRITE : READ_ONLY;
        int latency;

        log << "Address " << i << ": ("
            << segNum << ", "
            << pageNum << ", "
            << offset << ", "
            << (access == READ_ONLY ? "Read" : "Write") << ") -> ";

        int addr = st.translateAddress(segNum, pageNum, offset, access, latency);

        if (addr == -1) {
            faults++;
            log << "Failed\n";
        } else {
            log << "Physical=" << addr << ", Latency=" << latency << "\n";
        }
    }

    log << "Page Fault Rate: " << (double)faults / num * 100.0 << "%\n";
    log.close();
}

int main() {
    srand(time(0));

    int numFrames, pageSize, numSegments;

    cout << "Enter number of physical frames: ";
    cin >> numFrames;

    cout << "Enter page size: ";
    cin >> pageSize;

    cout << "Enter number of segments: ";
    cin >> numSegments;

    SegmentTable segmentTable(numFrames);

    for (int i = 0; i < numSegments; ++i) {
        int base = i * 10000;
        int limit = 3 + rand() % 5;
        Protection prot = (rand() % 2) ? READ_ONLY : READ_WRITE;
        segmentTable.addSegment(i, base, limit, prot, pageSize);
    }

    segmentTable.printMemoryMap();

    cout << "\nEnter logical address (seg, page, offset, access[0=read,1=write]) or -1 to stop:\n";

    while (true) {
        int segNum;
        cin >> segNum;

        if (segNum == -1) {
            break;
        }

        int pageNum, offset, access;
        cin >> pageNum >> offset >> access;

        int latency;
        int physicalAddress = segmentTable.translateAddress(
            segNum,
            pageNum,
            offset,
            access ? READ_WRITE : READ_ONLY,
            latency
        );

        if (physicalAddress != -1) {
            cout << "Physical Address: " << physicalAddress
                 << ", Latency: " << latency << "\n";
        }

        segmentTable.printMemoryMap();
        cout << "\nEnter next address or -1:\n";
    }

    cout << "Generate random addresses? (y/n): ";
    char genRand;
    cin >> genRand;

    if (genRand == 'y' || genRand == 'Y') {
        generateRandomAddresses(segmentTable, 100, 0.7, "results.txt");
        cout << "Results logged to results.txt\n";
    }

    return 0;
}