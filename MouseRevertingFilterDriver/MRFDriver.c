/*
* Part of the MouseRevertingFilterDriver
* ======================================
* the sceleton of the legacy driver
*/

#include "MRFDriver.h"

ULONG pendingkey = 0; // COUNTER FOR NOT FINISHED IRQLs

VOID DetachListOfDevices(PDEVICE_OBJECT DeviceObject) {
	while (DeviceObject) {

		IoDetachDevice(((PDEVICE_EXTENSION)DeviceObject->DeviceExtension)->lowerKbdDevice);
		DeviceObject = DeviceObject->NextDevice;
	}
}


VOID DriverUnload(PDRIVER_OBJECT DriverObject)
{
	LARGE_INTEGER interval = { 0 };
	PDEVICE_OBJECT DeviceObject = DriverObject->DeviceObject;
	interval.QuadPart = -10 * 1000 * 1000;

	DetachListOfDevices(DeviceObject);


	while (pendingkey) {
		KeDelayExecutionThread(KernelMode, FALSE, &interval);
	}

	DeviceObject = DriverObject->DeviceObject;
	while (DeviceObject) {
		IoDeleteDevice(DeviceObject);
		DeviceObject = DeviceObject->NextDevice;
	}

	KdPrint(("Unload MouseFilterDriver \r\n"));
}

NTSTATUS DispatchPass(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	IoCopyCurrentIrpStackLocationToNext(Irp);
	return IoCallDriver(((PDEVICE_EXTENSION)DeviceObject->DeviceExtension)->lowerKbdDevice, Irp);
}

NTSTATUS MyAttachDevice(PDRIVER_OBJECT DriverObject)
{
	NTSTATUS status;
	UNICODE_STRING MouseClassName = RTL_CONSTANT_STRING(L"\\Driver\\Mouclass");
	PDRIVER_OBJECT targetDriverObject = NULL;
	PDEVICE_OBJECT currentDeviceObject = NULL;
	PDEVICE_OBJECT myDeviceObject = NULL;

	status = ObReferenceObjectByName(&MouseClassName, OBJ_CASE_INSENSITIVE, NULL, 0, *IoDriverObjectType, KernelMode, NULL, (PVOID*)&targetDriverObject);
	if (!NT_SUCCESS(status)) {
		KdPrint(("ObReference  is failed \r\n"));
		return status;
	}

	ObDereferenceObject(targetDriverObject);

	currentDeviceObject = targetDriverObject->DeviceObject;

	while (currentDeviceObject != NULL) {
		status = IoCreateDevice(DriverObject, sizeof(DEVICE_EXTENSION), NULL, FILE_DEVICE_MOUSE, 0, FALSE, &myDeviceObject);
		if (!NT_SUCCESS(status)) {
			DetachListOfDevices(currentDeviceObject);
			return status;
		}

		RtlZeroMemory(myDeviceObject->DeviceExtension, sizeof(DEVICE_EXTENSION));
		status = IoAttachDeviceToDeviceStackSafe(myDeviceObject, currentDeviceObject, &((PDEVICE_EXTENSION)myDeviceObject->DeviceExtension)->lowerKbdDevice);

		if (!NT_SUCCESS(status)) {
			DetachListOfDevices(currentDeviceObject);
			return status;
		}

		myDeviceObject->Flags |= DO_BUFFERED_IO;
		myDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

		currentDeviceObject = currentDeviceObject->NextDevice;
	}


	return STATUS_SUCCESS;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	NTSTATUS status;
	int i;
	DriverObject->DriverUnload = DriverUnload;

	for (i = 1; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {
		DriverObject->MajorFunction[i] = DispatchPass;
	}

	DriverObject->MajorFunction[IRP_MJ_READ] = DispatchRead;

	status = MyAttachDevice(DriverObject);
	if (!NT_SUCCESS(status)) {
		KdPrint(("Attaching Mouse_Filter_Driver is failed \r\n"));

	}
	else {
		KdPrint(("Attacning Mouse_Filter_Driver is succeeds \r\n"));
	}
	return status;
}

