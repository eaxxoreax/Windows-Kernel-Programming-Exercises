/*
* This driver represents the text Chapter 3 of Pavel Yosifovich's book, Windows Kernel Programming
* 
* Some tips from my efforts:
*  - When allocating memory, it is prefered to use PagedPool vs NonPagedPool, reserving the use on non-paged
*    memory for when it is truly needed. 
*  - Use memory pool tags to track memory leaks if they occur
*  - Most vital, free all resources previously allocated, or memory leak will occur
*  - While not used in this code (we'll se it next chapter), the DriverObject will need to initialize a 
*    member DeviceObject to act as the interface with user-mode code
*  - The DriveObject will also need to initialize which dispatch routines are available for use in the driver
* 
*
* 
*/

#include <ntddk.h>

//Define a driver tag (reversed due to little-endian nature, will be view correctly in Poolmon
#define DRIVER_TAG 'dekw'

UNICODE_STRING g_RegistryPath;

//Driver must be unloaded, or else a memory leak occurs which will not be corrected until next reboot
void WKDExerciseDriverUnload(_In_ PDRIVER_OBJECT DriverObject) {
	UNREFERENCED_PARAMETER(DriverObject);

	DbgPrint("Unloading WKDExerciseDriver....\n");

	ExFreePool(g_RegistryPath.Buffer);
	DbgPrint("Registry path memory resources freed\n");
}

extern "C"

//NTSTATUS Driver Entry is Main() for a driver
NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath) {
	DbgPrint("WKDExerciseDriver loaded\n");
	
	DriverObject->DriverUnload = WKDExerciseDriverUnload;

	g_RegistryPath.Buffer = (WCHAR*)ExAllocatePoolWithTag(PagedPool, RegistryPath->Length, DRIVER_TAG);

	if (g_RegistryPath.Buffer == nullptr) {
		DbgPrint("Failed to allocate memory for Registry Path Buffer\n");
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	g_RegistryPath.MaximumLength = RegistryPath->Length;
	RtlCopyUnicodeString(&g_RegistryPath, (PUNICODE_STRING)RegistryPath);

	DbgPrint("Copied registry path: %wZ\n", g_RegistryPath);

	return STATUS_SUCCESS;
}

