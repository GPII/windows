SetBatchLines -1
ListLines Off

SELECTED_MODE = %1%
TABLETMODESTATE_DESKTOPMODE := 0x0
TABLETMODESTATE_TABLETMODE := 0x1

TabletModeController_GetMode(TabletModeController, ByRef mode) {
    return DllCall(NumGet(NumGet(TabletModeController+0),3*A_PtrSize), "Ptr", TabletModeController, "UInt*", mode)
}

TabletModeController_SetMode(TabletModeController, _TABLETMODESTATE, _TMCTRIGGER := 4) {
    return DllCall(NumGet(NumGet(TabletModeController+0),4*A_PtrSize), "Ptr", TabletModeController, "UInt", _TABLETMODESTATE, "UInt", _TMCTRIGGER)
}

ImmersiveShell := ComObjCreate("{C2F03A33-21F5-47FA-B4BB-156362A2F239}", "{00000000-0000-0000-C000-000000000046}")
TabletModeController := ComObjQuery(ImmersiveShell, "{4fda780a-acd2-41f7-b4f2-ebe674c9bf2a}", "{4fda780a-acd2-41f7-b4f2-ebe674c9bf2a}")

if (TabletModeController_GetMode(TabletModeController, mode) == 0)
    TabletModeController_SetMode(TabletModeController, SELECTED_MODE == TABLETMODESTATE_DESKTOPMODE ? TABLETMODESTATE_TABLETMODE : TABLETMODESTATE_DESKTOPMODE)

ObjRelease(TabletModeController), TabletModeController := 0
ObjRelease(ImmersiveShell), ImmersiveShell := 0 ; Can be freed after TabletModeController is created, instead
