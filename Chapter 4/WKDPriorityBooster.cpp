/*
* This driver represents the text Chapter 4 of Pavel Yosifovich's book, Windows Kernel Programming
*
* Some tips from my efforts:
*  - Driver must do these basic things:
*     - Set unload routine
*     - Set supported dispatch routines
*     - Create Device Object (be sure to delete in unload routine)
*     - Create symbolic link (be sure to remove symbolic link in unload routine)
*  - Code described in the book did not reference RegistryPath in DriverEntry. Be sure to use the UNREFERENCED_PARAMETER macro
*  - Inlude ntifs.h before ntddk.h, otherwise you receive an error complaining about redefining PETHREAD
*  - Code in the book does not reference DeviceObject in WKDPriorityBoosterDeviceControl. Use the UNREFERENCED_PARAMETER macro
*  - Not clear in the book, when declaring a pointer inside a case, the case must be scoped by open/close brackets. Since we
     declare data as a (ThreadData*) pointer inside our first case, e.g.
*     case IOCTL_WKD_PRIORITY_BOOSTER_SET_PRIORITY:
*     {
*       ...
*     }
*     default:
*  - As always, any dynamically allocated memory must be destroyed to prevent memory leaks (DeviceOobject, symLink, and thread)
*
*
*/

#include <ntifs.h>
#include <ntddk.h>
#include "WKDPriorityBoosterCommon.h"

//Prototypes
void WKDPriorityBoosterUnload(_In_ PDRIVER_OBJECT DriverObject);
NTSTATUS WKDPriorityBoosterCreateClose(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);
NTSTATUS WKDPriorityBoosterDeviceControl(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp); 

//Driver Entry

extern "C"

//NTSTATUS Driver Entry is Main() for a driver
NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath) {
	UNREFERENCED_PARAMETER(RegistryPath);

	DbgPrint("WKDPriorityBooster loaded\n");

	//Set Unload Routine
	DriverObject->DriverUnload = WKDPriorityBoosterUnload;

	//Set supported dispatch routines
	DriverObject->MajorFunction[IRP_MJ_CREATE] = WKDPriorityBoosterCreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = WKDPriorityBoosterCreateClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = WKDPriorityBoosterDeviceControl;

	//Create Device Object
	UNICODE_STRING deviceName = RTL_CONSTANT_STRING(L"\\Device\\WKDPriorityBooster");
	//Alternative option:
	//RtlInitUnicodeString(&deviceName, L"\\Device\\WKDPriorityBooster");

	PDEVICE_OBJECT DeviceObject;
	NTSTATUS status = IoCreateDevice(
		DriverObject,			//Our Driver Object
		0,						//No need for extra bytes
		&deviceName,			//Our Device Object's Name
		FILE_DEVICE_UNKNOWN,	//Type, used for software devices
		0,						//Chracteristics flag
		FALSE,					//Not exclusive
		&DeviceObject			//Pointer to resulting device object
	);

	if (!NT_SUCCESS(status)) {
		DbgPrint("Failed to create device object 0x%08X\n", status);
		return status;
	}

	//Create symbolic link to make driver accessable to user-mode code
	UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\WKDPriorityBooster");
	status = IoCreateSymbolicLink(&symLink, &deviceName);

	if (!NT_SUCCESS(status)) {
		DbgPrint("Failed to create symbolic link 0x%08X\n", status);
		return status;
	}

	return STATUS_SUCCESS;
}

//Function Implementations

//Driver must be unloaded, or else a memory leak occurs which will not be corrected until next reboot
void WKDPriorityBoosterUnload(_In_ PDRIVER_OBJECT DriverObject) {
	UNREFERENCED_PARAMETER(DriverObject);

	DbgPrint("Unloading WKDPriorityBooster....\n");

	//Delete Symbolic Link
	UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\WKDPriorityBooster");
	IoDeleteSymbolicLink(&symLink);

	//Delete Device Object
	IoDeleteDevice(DriverObject->DeviceObject);
}

//Driver Object Dispatch Routine handles both Create and Close events
_Use_decl_annotations_
NTSTATUS WKDPriorityBoosterCreateClose(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

//Driver Dispatch Routine handles change of thread priority
//Typical drivers may support many control codes, so fail if we don't recognize the code
_Use_decl_annotations_
NTSTATUS WKDPriorityBoosterDeviceControl(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);
	//Get our IO_STACK_LOCATION
	auto stack = IoGetCurrentIrpStackLocation(Irp);
	auto status = STATUS_SUCCESS;

	//Determine Control Code and handle each case
	switch (stack->Parameters.DeviceIoControl.IoControlCode) {
	case IOCTL_WKD_PRIORITY_BOOSTER_SET_PRIORITY:
	{
		//Check if the buffer we received is large enough to hold ThreadData structure
		if (stack->Parameters.DeviceIoControl.InputBufferLength < sizeof(ThreadData)) {
			status = STATUS_BUFFER_TOO_SMALL;
			break;
		}
		auto data = (ThreadData*)stack->Parameters.DeviceIoControl.Type3InputBuffer;

		//Check if the received buffer is null
		if (data == nullptr) {
			status = STATUS_INVALID_PARAMETER;
			break;
		}

		//Check if Priority is in valid range
		if (data->Priority < 1 || data->Priority > 31) {
			status = STATUS_INVALID_PARAMETER;
			break;
		}

		//Get thread by ID, then set thread priority
		PETHREAD thread;
		status = PsLookupThreadByThreadId(ULongToHandle(data->ThreadId), &thread);
		if (!NT_SUCCESS(status))
			break;

		KeSetPriorityThread((PKTHREAD)thread, data->Priority);

		//Dereference thread so we do not have a memory leak
		ObDereferenceObject(thread);

		break;
	}
	default:
		//Failed to recognize control code
		status = STATUS_INVALID_DEVICE_REQUEST;
		break;
	}

	//Complete the IO Request
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}