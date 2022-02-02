/*
* This driver represents the textand Exercise 1 for Chapter 1 of Pavel Yosifovich's book, Windows Kernel Programming
* 
* Some tips from my efforts:
*  - If testing on a 64-bit system, make sure the build settings for hte project are set to compile in 64-bit mode
*    In Visual Studio: Build->Configuration Manager-> Set dropdowns to x64
*  - Be sure to add the registry value described in Chapter 1 to make DbgPrint() actually show output
*  - You likely will need to disable driver signing on your test machine
*    in administrator command prompt: bcdedit /set nointegritychecks on
* 
*
* 
*/

#include <ntddk.h>

//Driver must be unloaded, or else a memory leak occurs which will not be corrected until next reboot
void WKDExerciseDriverUnload(_In_ PDRIVER_OBJECT DriverObject) {
	UNREFERENCED_PARAMETER(DriverObject);

	DbgPrint("Unloading WKDExerciseDriver....");
}

//Print out information returned from RtlGetVersion. This can be further parsed to print the exact version using reference
//at https://docs.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-osversioninfow
void PrintOSInfo(_In_ RTL_OSVERSIONINFOW& osVersion) {
	DbgPrint("Platform ID: %u", osVersion.dwPlatformId);
	DbgPrint("Major Version: %u", osVersion.dwMajorVersion);
	DbgPrint("Minor Version: %u", osVersion.dwMinorVersion);
	DbgPrint("Build Number: %u", osVersion.dwBuildNumber);
}

extern "C"

//NTSTATUS Driver Entry is Main() for a driver
NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath) {
	UNREFERENCED_PARAMETER(RegistryPath);
	DbgPrint("WKDExerciseDriver loaded");

	RTL_OSVERSIONINFOW osVersion;
	osVersion.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);
	NTSTATUS result = RtlGetVersion(&osVersion);

	//If RtlGetVersion failed, print an error, otherwise print the OS Version Information
	if (!NT_SUCCESS(result)) {
		DbgPrint("Error: 0x%08X\n", result);
		return result;
	}
	else {
		PrintOSInfo(osVersion);
	}
	
	DriverObject->DriverUnload = WKDExerciseDriverUnload;

	return STATUS_SUCCESS;
}

