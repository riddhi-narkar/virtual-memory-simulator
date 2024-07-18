#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <cstring>

using namespace std;

/*
The goal of this project was to write a program that translates
logical address to physical address for a virtual address space
of 2^16 (65,536) bytes. We read in the file containing logical
addresses, using a TLB (translation look-aside buffer) as well
as a page table, and translated each logical address to its
corresponding physical address and output the value of the byte
stored at the translated physical address. Then print out these
values and ratios (page fault rate and TLB rate).

To run:
g++ -o VirtualMemory VirtualMemory.cpp
./VirtualMemory addresses.txt
*/

ifstream backStore;
ifstream addressFile;

#define LINELENGTH 10

#define PAGESIZE 256
vector<int> pageTable(PAGESIZE, -1);
vector<int> pageFrame(PAGESIZE, -1);

#define TLB_LENGTH 16
vector<int> TLBPage(TLB_LENGTH, -1);
vector<int> TLBFrame(TLB_LENGTH, -1);
int TLBNum = 0;
int TLBCounter = 0;

#define FRAMELENGTH 256
vector<char> readBacker(FRAMELENGTH, 0);

#define physicalMemoryBytes 65536 //2^16
vector<int> physicalMemory(physicalMemoryBytes, 0);
int pageFault = 0;


/* Receives the page and reads from the BACK_STORE file and into the
readBacker array. We then get the available frame and go through
the entire page size (256) and insert the info into the physical
memory array. Next we insert the frame into the page table and
increase the page faults. Finally we return the frame we used. */
int readBackStore(int page) {
    int availableFrame = 0;

    /* SEEK_SET is in fseek() - it seeks from the beginning of the file */
    backStore.seekg(page * PAGESIZE, ios::beg);
    if (!backStore.read(&readBacker[0], PAGESIZE)) {
        cerr << "ERROR reading from backStore" << endl;
    }

    /* Get available frame by looking for unused index in pageFrame */
    for (int i = 0; i < PAGESIZE; i++) {
        if (pageFrame[i] == -1) {
            pageFrame[i] = 0;
            availableFrame = i;
            break;
        }
    }

    /* Start at specific index for each frame */
    int startFrameIndex = PAGESIZE * availableFrame;
    for (int j = 0; j < PAGESIZE; j++) {
        physicalMemory[startFrameIndex] = readBacker[j];
        startFrameIndex++;
    }

    pageTable[page] = availableFrame;
    pageFault++;

    return availableFrame;
}

/* Gets the page and the offset of the logical address, checks if it's
in the TLB page, if it is, save that frame. If not, then either reads
the page from the page frame and saves the available frame to be used
to get info from the backstore into the physical memory array (this is
a page fault). Then the info is inserted into the TLB page and TLB frame.
We then return the physical memory address. */
int changeAddress(int logAddress) {
    int page = logAddress / PAGESIZE;
    double offsetDub = fmod(static_cast<double>(logAddress), static_cast<double>(PAGESIZE));
    int offset = static_cast<int>(offsetDub);
    int frameNum = -1;

    /* check if page is in TLB frame */
    for (int i = 0; i < TLB_LENGTH; i++) {
        if (TLBPage[i] == page) {
            frameNum = TLBFrame[i];
            TLBNum++;
            break;
        }
    }

    /* if page was not in TLB, read from BACK_STORE, or
    get page from pageTable */
    if (frameNum == -1) {
        /* if not in either, page fault */
        if (pageTable[page] == -1) {
            frameNum = readBackStore(page);
        } else {
            /* if not in TLB frame, get from pageTable */
            frameNum = pageTable[page];
        }

        TLBPage[TLBCounter % TLB_LENGTH] = page;
        TLBFrame[TLBCounter % TLB_LENGTH] = frameNum;
        TLBCounter++;
    }

    return (frameNum * PAGESIZE) + offset;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cerr << "Please enter two arguments.\nEx: ./file addresses.txt" << endl;
        return -1;
    }

    /* Open Files */
    backStore.open("BACKING_STORE.bin", ios::in | ios::binary);
    if (!backStore) {
        cerr << "Error opening BACKING_STORE.bin" << endl;
        return -1;
    }

    addressFile.open(argv[1], ios::in);
    if (!addressFile) {
        cerr << "Error opening address file" << endl;
        return -1;
    }

    /* Initialize variables */
    int translations = 0;
    string line;

    /* Go through each line of address file and pass logical address
    to Change address, which will translate the info to a physical address */
    while (getline(addressFile, line)) {
        int logAddress = stoi(line);
        int address = changeAddress(logAddress);

        cout << "Logical Address: " << logAddress
             << ", Physical Memory: " << address
             << ", Value: " << physicalMemory[address] << endl;
        translations++;
    }

    /* Print out results */
    cout << "\n*** Final Info ***" << endl;
    cout << "Number of translations: " << translations << endl;
    cout << "Number of Page Faults: " << pageFault << endl;
    cout << "Page Fault Rate: " << static_cast<float>(pageFault) / translations << endl;
    cout << "Number of TLB Hits: " << TLBNum << endl;
    cout << "TLB Rate: " << static_cast<float>(TLBNum) / translations << endl;

    /* Close files */
    addressFile.close();
    backStore.close();

    return 0;
}
