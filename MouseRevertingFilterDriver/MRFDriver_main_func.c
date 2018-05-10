/*
*  Part of the MouseRevertingFilterDriver (legacy driver)
* ===========================
* main functional routines
*/
#include "MRFDriver.h"

extern ULONG pendingkey; // COUNTER FOR NOT FINISHED IRQLs

static
ULONG SWITCH_SIGNALS_SEQUENCE[6] = { MOUSE_RIGHT_BUTTON_DOWN, MOUSE_RIGHT_BUTTON_UP,
						MOUSE_RIGHT_BUTTON_DOWN, MOUSE_RIGHT_BUTTON_UP,
						MOUSE_RIGHT_BUTTON_DOWN, MOUSE_RIGHT_BUTTON_UP };

#define LENGTH_SWITCH_SEQUENSE 6

static
ULONG CurrentWaitingSignal = 0;

static
BOOLEAN inversion = FALSE; 

#define MAX_MOUSE_Y_VALUE  0xFFFF

NTSTATUS ReadComplete(PDEVICE_OBJECT DeviceObject, PIRP Irp, PVOID Context)
{  
	PMOUSE_INPUT_DATA pMouseData = (PMOUSE_INPUT_DATA)Irp->AssociatedIrp.SystemBuffer;
	int structnum = Irp->IoStatus.Information / sizeof(MOUSE_INPUT_DATA);
	
	if (Irp->IoStatus.Status == STATUS_SUCCESS) {

		for (int i = 0; i < structnum; i++) {

			ULONG flags = pMouseData[i].ButtonFlags;

			KdPrint(("flags = %d, curWaitingBut= %d \r\n", flags, CurrentWaitingSignal));

			if (flags == SWITCH_SIGNALS_SEQUENCE[CurrentWaitingSignal]) {

				CurrentWaitingSignal++;

				if (CurrentWaitingSignal == LENGTH_SWITCH_SEQUENSE ) {
					inversion = ! inversion;
				}
			}
			else {
				CurrentWaitingSignal = 0;
			}	

			if (inversion) {

				// Here should be routines for valid proceeding for MOUSE_MOVE_ABSOLUTE but, as it was not checked, only RELATIVE is keeping
				
				//if ((flags & MOUSE_MOVE_ABSOLUTE) != 0) { // can't check
				//	pMouseData[i].LastY = -pMouseData[i].LastY;
				//} 
				//else {
					pMouseData[i].LastY = MAX_MOUSE_Y_VALUE - pMouseData[i].LastY;
				//}

			}
		}
	}

	if (Irp->PendingReturned) {
		IoMarkIrpPending(Irp);
	}

	pendingkey--;

	return Irp->IoStatus.Status;
}

NTSTATUS DispatchRead(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{

	IoCopyCurrentIrpStackLocationToNext(Irp); 

	IoSetCompletionRoutine(Irp, ReadComplete, NULL, TRUE, TRUE, TRUE);

	pendingkey++;

	NTSTATUS status = IoCallDriver(((PDEVICE_EXTENSION)DeviceObject->DeviceExtension)->lowerKbdDevice, Irp);

	return status;
}
