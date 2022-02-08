// BoosterClient.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include <stdio.h>
#include "..\PriorityBooster\WKDPriorityBoosterCommon.h"

int main(int argc, const char* argv[])
{
    //Check for command line arguments
    if (argc < 3) {
        printf("Usage: BoosterClient <threadId> <priority> \n");
        return 0;
    }

    //Open handle to device driver
    HANDLE hDevice = CreateFile(L"\\\\.\\WKDPriorityBooster", GENERIC_WRITE,
        FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);

    if (hDevice == INVALID_HANDLE_VALUE) {
        printf("Failed to open device (error=%d)", GetLastError());
        return 1;
    }

    //Get thread data from command line. Caution! No input validation implemented here!
    ThreadData data;
    data.ThreadId = atoi(argv[1]); //First command line argument
    data.Priority = atoi(argv[2]); //Second command line argument

    DWORD returned;
    BOOL success = DeviceIoControl(hDevice,         //Device
        IOCTL_WKD_PRIORITY_BOOSTER_SET_PRIORITY,    //Control Code
        &data, sizeof(data),                        //Input buffer and length
        nullptr, 0,                                 //Output buffer and length
        &returned, nullptr);

    if (success)
        printf("Priority change succeded!\n");
    else
        printf("Priority change failed! (error=%d)\n", GetLastError());

    CloseHandle(hDevice);
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
