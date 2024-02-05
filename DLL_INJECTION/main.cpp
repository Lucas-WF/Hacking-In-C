#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <winbase.h>
#include <fstream>

DWORD getPID(const wchar_t* procName);

int main(int argc, char* argv[]) {
    const wchar_t* proc = L"Notepad.exe";

    /*
    if(argv[1] != NULL) {
         proc = argv[1];
    } */

    DWORD pid = getPID(proc);

    if (pid == 0) {
        std::cout << "No PID was found!" << "\n";
        std::cin.get();
        return 1;
    }

    wchar_t DllpathC[MAX_PATH];
    DWORD result = GetCurrentDirectory(sizeof(DllpathC) / sizeof(DllpathC[0]), DllpathC);

    if (result == 0) {
        std::cout << "Fail" << "\n";
        std::cin.get();
        return 1;
    }

    wcscat_s(DllpathC, L"\\DLL_Test.dll");

    std::ifstream file;

    file.open(DllpathC);

    if (!file) {
        std::cout << "Error when openning the DLL" << "\n";
        std::cin.get();
        return 1;
    }

    std::wcout << "DLL path: " << DllpathC << "\n";

    /*
    if(argv[2] != NULL) {
        DllpathC = argv[2];
    }
    */

    // Open the process and return a handle to it
    HANDLE hProc = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_QUERY_INFORMATION, FALSE, pid);

    if (hProc == NULL) {
        std::cout << "Error when openning the Proccess" << "\n";
        std::cin.get();
        return 1;
    }

    BOOL is_target64;
    IsWow64Process(hProc, &is_target64);

    BOOL is_injector64 = FALSE;
    IsWow64Process(GetCurrentProcess(), &is_injector64);

    if (is_target64 != is_injector64) {
        CloseHandle(hProc);
        std::cout << "Process mismatching!" << "\n";
        std::cin.get();
        return 1;
    }


    HMODULE hModule = GetModuleHandleW(L"kernel32.dll");

    if (hModule == NULL) {
        CloseHandle(hProc);
        std::cout << "Error on getting the kernel32" << GetLastError() << "\n";
        std::cin.get();
        return 1;
    }

    FARPROC hLoadLib = GetProcAddress(hModule, "LoadLibraryW");

    if (hLoadLib == NULL) {
        CloseHandle(hProc);
        std::cout << "Error on getting the LoadLibraryW" << GetLastError() << "\n";
        std::cin.get();
        return 1;
    }

    SIZE_T path_size = (wcslen(DllpathC) * sizeof(DllpathC[0]));


    /* Reserves, commits, or changes the state of a region of memory within the virtual address space of a specified process.
    In this case, it commits.*/
    LPVOID pBaseAddr = VirtualAllocEx(hProc, NULL, path_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

    if (pBaseAddr == NULL) {
        CloseHandle(hProc);
        std::cout << "Error on commiting the memory" << GetLastError() << "\n";
        std::cin.get();
        return 1;
    }

    std::cout << "Load at: " << pBaseAddr << "\n";

    // Writes data to an area of memory in a specified process
    if (WriteProcessMemory(hProc, pBaseAddr, DllpathC, path_size, NULL) == 0) {
        VirtualFreeEx(hProc, pBaseAddr, 0, MEM_RELEASE);
        CloseHandle(hProc);
        std::cout << "Error on writing data to the memory" << GetLastError() << "\n";
        std::cin.get();
        return 1;
    }

    HANDLE hLoadThread = CreateRemoteThread(hProc, NULL, 0, (LPTHREAD_START_ROUTINE)hLoadLib, pBaseAddr, 0, NULL);

    if (hLoadThread == NULL) {
        VirtualFreeEx(hProc, pBaseAddr, 0, MEM_RELEASE);
        CloseHandle(hProc);
        std::cout << "Error when creating a thread" << GetLastError() << "\n";
        std::cin.get();
        return 1;
    }

    std::cout << "Loading library\n";

    WaitForSingleObject(hLoadThread, INFINITE);

    std::cin.get();

    CloseHandle(hLoadThread);
    VirtualFreeEx(hProc, pBaseAddr, 0, MEM_RELEASE);
    CloseHandle(hProc);
}

DWORD getPID(const wchar_t* procName) {
    // Takes a Snapshot of the current specified processes (in this case all of them) 
    HANDLE hProcSnap = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);

    // Describes an entry from a list of the processes residing in the system address space when a snapshot was taken
    PROCESSENTRY32 pe32;

    /* The size of the structure, in bytes. Before calling the
    Process32First function, set this member to sizeof(PROCESSENTRY32). If you do not initialize dwSize, Process32First fails. */
    if (hProcSnap == INVALID_HANDLE_VALUE) {
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
        std::wcout << pe32.szExeFile;
        std::cout << ": " << pe32.th32ProcessID << "\n";
        // Check if the process name matches (The pe32 isn't a wchar* in g++)
        if (_wcsicmp(pe32.szExeFile, procName) == 0) {
            break;
        }
    } while (Process32Next(hProcSnap, &pe32));

    CloseHandle(hProcSnap);
    return pe32.th32ProcessID;
    return 0;
}