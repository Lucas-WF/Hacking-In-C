#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>

DWORD getPID(const char* procName);

int main(int argc, char* argv[]) {
    const char* proc = "running.exe";

    if(argv[1] != NULL) {
         proc = argv[1];
    }

    DWORD pid = getPID(proc);

    if(pid == 0) {
        fprintf(stderr ,"No PID was found!");
        getchar();
        return 1;
    }

    LPCSTR DllpathC = "FirstDLL.dll";

    if(argv[2] != NULL) {
        DllpathC = argv[2];
    }

    // Open the process and return a handle to it
    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

    if(hProc == NULL) {
        fprintf(stderr ,"Error when openning the Proccess");
        getchar();
        return 1;
    }
    
    /* Reserves, commits, or changes the state of a region of memory within the virtual address space of a specified process.
    In this case, it commits.*/
    LPVOID pBaseAddr = VirtualAllocEx(hProc, 0, strlen(DllpathC) + 1, MEM_COMMIT, PAGE_READWRITE);

    if(pBaseAddr == NULL) {
        fprintf(stderr ,"Error on commiting the memory");
        getchar();
        return 1;
    }

    // Writes data to an area of memory in a specified process
    if(WriteProcessMemory(hProc, pBaseAddr, (LPVOID)DllpathC, strlen(DllpathC) + 1, NULL) == 0) {
        fprintf(stderr ,"Error on writing data to the memory");
        getchar();
        return 1;
    }

    HANDLE hLoadThread = CreateRemoteThread(hProc, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibrary(DllpathC), pBaseAddr, 0, NULL);

    if(hLoadThread == NULL) {
        fprintf(stderr ,"Error when creating a thread");
        getchar();
        return 1;
    }

    WaitForSingleObject(hLoadThread, INFINITE);

    std::cin.get();

    CloseHandle(hLoadThread);
    VirtualFreeEx(hProc, pBaseAddr, 0, MEM_RELEASE);
    CloseHandle(hProc);
}

DWORD getPID(const char* procName) {
    // Takes a Snapshot of the current specified processes (in this case all of them) 
    HANDLE hProcSnap = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);

    // Describes an entry from a list of the processes residing in the system address space when a snapshot was taken
    PROCESSENTRY32 pe32;
    
    /* The size of the structure, in bytes. Before calling the 
    Process32First function, set this member to sizeof(PROCESSENTRY32). If you do not initialize dwSize, Process32First fails. */
    if(hProcSnap == INVALID_HANDLE_VALUE) {
        return 0;
    }

    /* The size of the structure, in bytes. Before calling the 
    Process32First function, set this member to sizeof(PROCESSENTRY32). If you do not initialize dwSize, Process32First fails. */
    pe32.dwSize = sizeof(PROCESSENTRY32);
    
    /* Recover the information about the first proccess. If the entry has been copied to the buffer, it will return true. Else it
    will return false */ 
    if (!Process32First(hProcSnap, &pe32)) {
        CloseHandle(hProcSnap);
        return 0;
    }

    do {
        std::cout << pe32.szExeFile << ": " << pe32.th32ProcessID << std::endl;
        // Check if the process name matches (The pe32 isn't a wchar* in g++)
        if (strcmp(pe32.szExeFile, procName) == 0) {
            CloseHandle(hProcSnap);
            return pe32.th32ProcessID;
        }
    } while (Process32Next(hProcSnap, &pe32));

    CloseHandle(hProcSnap);
    return 0;
}