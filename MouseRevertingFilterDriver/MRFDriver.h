/*
* This is a legacy filter driver   MouseRevertingFilterDriver 
* files: MRFDriver.h, MRFDriver.c, MRFDriver_main_func.c
* ========================================
* Functionality:
* Reversing Vertical mouse moving
* ========================================
* installation by: 
* -> sc create MouseFilter type= kernel binPath= C:\drivers\MouseRevertingFilterDriver.sys
* -> net start MouseFilter 
* ========================================
* using: 
* triple click right button turn on reversing
* triple click right button turn reversing back 
*/

#include <ntddk.h>
#include "Ntddmou.h" //special flags & _MOUSE_INPUT_DATA


// Declaration of main functional routines (MRFDriver_main_func.c)
NTSTATUS ReadComplete(PDEVICE_OBJECT DeviceObject, PIRP Irp, PVOID Context);
NTSTATUS DispatchRead(PDEVICE_OBJECT DeviceObject, PIRP Irp);

// This is absence in standard headers
NTSTATUS ObReferenceObjectByName(
	IN PUNICODE_STRING ObjectName,
	IN ULONG Attributes,
	IN PACCESS_STATE AccessState,
	IN ACCESS_MASK DesiredAccess,
	IN POBJECT_TYPE ObjectType,
	IN KPROCESSOR_MODE ACCESSMode,
	IN PVOID ParseContext,
	OUT PVOID *Object
);

// MyDevice Extention
typedef struct {
	PDEVICE_OBJECT lowerKbdDevice;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

extern POBJECT_TYPE *IoDriverObjectType;
