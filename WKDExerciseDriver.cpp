#include <ntddk.h>

//Driver must be unloaded, or else a memory leak occurs which will not be corrected until next reboot
void WKDExerciseDriverUnload(_In_ PDRIVER_OBJECT DriverObject) {
	UNREFERENCED_PARAMETER(DriverObject);
}

extern "C"

//NTSTATUS Driver Entry is Main() for a driver
NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath) {
	UNREFERENCED_PARAMETER(RegistryPath);

	DriverObject->DriverUnload = WKDExerciseDriverUnload;

	return STATUS_SUCCESS;
}

