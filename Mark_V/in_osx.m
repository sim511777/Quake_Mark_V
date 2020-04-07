
//
// "in_osx.m" - MacOS X mouse driver
//
// Written by:  Axel 'awe' Wefers           [mailto:awe@fruitz-of-dojo.de].
//              �2001-2012 Fruitz Of Dojo   [http://www.fruitz-of-dojo.de].
//
// Quake is copyrighted by id software     [http://www.idsoftware.com].
//
// Version History:
// v1.2.0: Rewritten. Uses now IOHIDManager.
//         Added support for game pads.
//         Added support for actuators.
// v1.0.8: F12 eject is now disabled while Quake is running.
// v1.0.6: Mouse sensitivity works now as expected.
// v1.0.5: Reworked whole mouse handling code [required for windowed mouse].
// v1.0.0: Initial release.
//


#import <AppKit/AppKit.h>
#import <IOKit/IOKitLib.h>
#import <IOKit/hidsystem/IOHIDLib.h>
#import <IOKit/hid/IOHIDLib.h>



#import "quakedef.h"
#import "macquake.h"



typedef struct
{
    SInt32  X;
    SInt32  Y;
} in_mousepos_t;




static in_mousepos_t            sInMouseNewPosition     = { 0 };
static in_mousepos_t            sInMouseOldPosition     = { 0 };

// Baker: I don't like making a function just for this.
// However, it may become important for mouse drag operations in future
void Input_Local_Deactivate (void)
{
    [sysplat.gVidWindow dragMoveEnded]; // Baker: Reset the drag even if we don't have the mouse
}

int Input_Local_Capture_Mouse (cbool bDoCapture)
{
    static cbool captured = false;
#pragma message ("Baker: To do, fail the capture if cursor is above client area and windowed")
    if (bDoCapture && !captured)
    {
        if (![sysplat.gVidWindow captureMouse: YES]) // Shows mouse cursor
        {
//          Con_Printf ("Capture rejected");
            return 0; // Capture rejected
        }

        Con_DPrintf ("Mouse Captured\n");
        captured = true;
    }
    
    if (!bDoCapture && captured)
    {
        [sysplat.gVidWindow captureMouse: NO]; // Hides mouse cursor
        [sysplat.gVidWindow dragMoveEnded]; // Baker: By definition, if we capture
// Baker: Windows does a release capture and unclips cursor move
        Con_DPrintf ("Mouse Released\n");
        captured = false;
    }

    return 1; // Accepted
}



void Input_Local_Init (void)
{
    NSArray* devices = [NSArray arrayWithObjects: FDHIDDeviceGamePad, FDHIDDeviceKeyboard, FDHIDDeviceMouse, nil];
    FDHIDManager* sharedInput = [FDHIDManager sharedHIDManager];

    if (!sharedInput)
        System_Error ("Failed to open HID manager!");
        
    [sharedInput setDeviceFilter: devices];
}



void Input_Local_Shutdown (void)
{
     
    [[FDHIDManager sharedHIDManager] release];
}

// Baker: On OS X these occur same place as mouse
void Input_Local_Joystick_Commands (void) { }

cbool Input_Local_Joystick_Startup (void) { return true; } // Baker I guess

void Input_Mouse_Move (usercmd_t *cmd)
{
    int mouse_x, mouse_y;


    if ([sysplat.gVidWindow isMiniaturized])
        return;

//  if (vid.screen.type == MODE_WINDOWED)
//      return;

    mouse_x = sInMouseNewPosition.X;
    mouse_y = sInMouseNewPosition.Y;

    sInMouseOldPosition     = sInMouseNewPosition;
    sInMouseNewPosition.X   = 0;
    sInMouseNewPosition.Y   = 0;
    


    if (mouse_x || mouse_y)
    {
        mouse_x *= sensitivity.value;
        mouse_y *= sensitivity.value;
    
        if ((in_strafe.state & 1) || (lookstrafe.value && MOUSELOOK_ACTIVE))
            cmd->sidemove += m_side.value * mouse_x;
        else cl.viewangles[YAW] -= m_yaw.value * mouse_x;
                    
        if (MOUSELOOK_ACTIVE) 
            View_StopPitchDrift ();
                
        if (MOUSELOOK_ACTIVE && !(in_strafe.state & 1))
        {
            cl.viewangles[PITCH] += m_pitch.value * mouse_y;
            
            CL_BoundViewPitch (cl.viewangles); // Keyboard lock
    
    //      cl.viewangles[PITCH] = CLAMP (cl_minpitch.value, cl.viewangles[PITCH], cl_maxpitch.value);
        }
        else
        {
            if ((in_strafe.state & 1) && cl.noclip_anglehack)
                cmd->upmove -= m_forward.value * mouse_y;
            else cmd->forwardmove -= m_forward.value * mouse_y;
        }
    }
}

float sInJoyValues[JOY_MAX_AXES] = { 0 };

#pragma message ("Baker: This function needs a better name if does both mouse/joy")


void Input_Local_SendKeyEvents (void)
{
    const FDHIDEvent*   pEvent      = nil;
#pragma message ("Baker: This needs to be more picky about when/how to accept joystick events i.e. must be active, etc.")
    const BOOL allowJoy    = /* joy_avail &&*/ in_joystick.value;
    const BOOL allowMouse  = vid.ActiveApp && !vid.Hidden && !vid.Minimized && (vid.screen.type == MODE_FULLSCREEN || m_keys_bind_grab || (key_dest == key_game && !console1.forcedup));
    BOOL gotmouse = allowMouse;
      if (!Input_Local_Capture_Mouse (allowMouse))
          gotmouse = NO;

#pragma message ("Baker: The above statement is exactly where we need to check for drag end")
#pragma message ("Baker: And enable mouse wheel support")
    
    
    while ((pEvent = [[FDHIDManager sharedHIDManager] nextEvent]) != nil)
    {
        switch (pEvent->mType)
        {
            case eFDHIDEventTypeGamePadAxis:
                if (allowJoy && pEvent->mButton < JOY_MAX_AXES)
                    sInJoyValues[pEvent->mButton] = pEvent->mFloatVal;
                break;
                
            case eFDHIDEventTypeGamePadButton:
                if (allowJoy && pEvent->mButton <= (K_AUX32 - K_AUX1))
                    Key_Event (K_AUX1 + pEvent->mButton, pEvent->mBoolVal);
                break;
                
            case eFDHIDEventTypeKeyboard:
                if (pEvent->mButton < K_PAUSE /*Baker: which is 255 */ )
                    Key_Event (pEvent->mButton, pEvent->mBoolVal);
                break;
                
            case eFDHIDEventTypeMouseAxis:
                
                switch (pEvent->mButton)
                {
                case eFDHIDMouseAxisX:
                    if (gotmouse) // Baker: Mouse can't move in a drag operation
                        sInMouseNewPosition.X += pEvent->mIntVal;
                    break;
                                    
                case eFDHIDMouseAxisY:
                    if (gotmouse) // Baker: Mouse can't move in a drag operation
                        sInMouseNewPosition.Y += pEvent->mIntVal;
                    break;
                        
                case eFDHIDMouseAxisWheel:
                    // Baker: this is fine even if gotmouse and allowmouse are false
                    // We still want the mouse wheel input
                    if (pEvent->mIntVal)
                    {
                        const int wheelEvent = (pEvent->mIntVal > 0) ? K_MOUSEWHEELUP : K_MOUSEWHEELDOWN;
                        
                        Key_Event (wheelEvent, true);
                        Key_Event (wheelEvent, false);
                    }
                    break;
                }
                break;
                
            case eFDHIDEventTypeMouseButton:
                if (!allowMouse || pEvent->mButton >= 5)
                    break;
                
                if (!gotmouse)
                {
                    // Baker: Detect if a drag is occuring.  A mouse1 release here tells us it is over
                    if (pEvent->mButton == 0 && !pEvent->mBoolVal)
                    {
                        // This should be a mouse1 release
                        if ([sysplat.gVidWindow inDragMove])
                        {
                            [sysplat.gVidWindow dragMoveEnded];
                            Con_Printf ("Drag move ended\n");
                        }
                    }
                } else Key_Event (K_MOUSE1 + pEvent->mButton, pEvent->mBoolVal);
                break;
        }
    }
    
//    Input_UpdateActuators ();
}
