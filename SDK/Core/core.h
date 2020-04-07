/*
Copyright (C) 2011-2014 Baker

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// core.h -- core functions

/* NOT True Core ...
core_opengl.h
gl_constants.h
libcurl.dll
math_matrix.c
math_vector.c
unzip_win.cpp
unzip_win.h
zip_win.cpp
zip_win.h
zlib1.dll
*/

#ifndef __CORE_H__
#define __CORE_H__

#ifdef __GNUC__
	#pragma GCC diagnostic ignored "-Wmissing-field-initializers" // Not a fan of disabling this but messes with struct x mystruct = {0};
#endif // __GNUC__

// #define CORE_LIBCURL

/*
** Allocations in Core SHALL be made with C allocators calloc, free, etc.
**
** Note that Corex and Corex_GL extensions to Core follow different rules.
*/


#define __CORE_INCLUDED__

// it's in environment.h
//#if defined (DEBUG) && !defined (_DEBUG)
//	#define _DEBUG
//#endif // Keep everything consistent across platforms

#include "environment.h"

#if defined(_CONSOLE) && defined(PLATFORM_OSX)
//	#include <CoreFoundation/CoreFoundation.h>
#endif
#if defined(PLATFORM_OSX) || defined(PLATFORM_IOS)
	#include <mach/mach_time.h>
#endif


#ifdef __OBJC__ // The Mac
	#ifndef _CONSOLE
		#ifdef PLATFORM_OSX
			#import <Cocoa/Cocoa.h> // core_mac.h sort of
			#import <ApplicationServices/ApplicationServices.h>
			#import "VidWindow.h"
		#endif

		#ifdef PLATFORM_IOS
			#import <GLKit/GLKit.h>
			#import <UIKit/UIKit.h>

			#import "VidUIWindow.h"
		#endif

	#endif
#endif

#include <stdio.h> // fopen, etc.
#include <stdlib.h> // malloc, etc.

//#include <math.h> // Environment.h already includes this for M_PI defined check
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h> // _stat, etc.
#include <time.h>
#include <ctype.h> // islower, etc.
#include <stddef.h> // offsetof
#include <stdarg.h> // GCC wanted this
#include <assert.h>

#define member_size(type, member) sizeof(((type *)0)->member)

#ifndef _MSC_VER // About everything else needs this header
    #ifdef PLATFORM_WINDOWS
        // MinGW doesn't have alloca.h so we will define it ourselves.
        void *alloca(size_t size);
    #else
        #include <alloca.h>
    #endif
#endif

#ifdef PLATFORM_WINDOWS
	#include <io.h>
	#include <direct.h> // _chdir, etc.
	#include "dirent_win.h" // dirent.h for Windows
#endif // PLATFORM_WINDOWS

#ifndef PLATFORM_WINDOWS
	#include <unistd.h> // read
#include <dirent.h>
#include <dlfcn.h>
	#include <fcntl.h>
#endif // ! PLATFORM_WINDOWS

#ifdef CORE_PTHREADS
#include "pthreads_core.h"
#endif // PTHREADS

#ifdef PLATFORM_ANDROID
	#include <android/log.h>
#endif // PLATFORM_ANDROID




#include "lists.h"
#include "file.h"
//#include "image.h" // Moved to below function declares ...
#include "interface.h"
#include "math_general.h"
#include "math_vector.h"
#include "math_matrix.h"
#include "pak.h"
#include "stringlib.h"
#include "base64.h"
#include "zip.h"
// #include "download.h" // Moved to below function declares
#include "enumbits.h"
#include "timelib.h"
#include "music.h"
#include "links.h"
#include "memchain.h"

///////////////////////////////////////////////////////////////////////////////
//  CORE: Basic function setup
///////////////////////////////////////////////////////////////////////////////


#define KEYMAP_COUNT_512 512
#define NUM_MOUSE_BUTTONS_5 5
extern int keymap [KEYMAP_COUNT_512][5]; // Decimal representation, Platform constant, Our constant, Special instruction, Special instruction value
extern keyvalue_t key_scancodes_table [108];

#define CORE_KEYMAP_EMISSION_2 2 		// The key event came from local interpretation (character emission), not scancode.
#define CORE_SHIFTBITS_UNREAD_NEG1 -1 	// We did not read the shiftbits, this value must be ignored by receiver.


#define K_SHIFT_MASK_1		1
#define K_CTRL_MASK_2		2
#define K_ALT_MASK_4		4
#define K_COMMAND_MASK_8	8

#define K_MOUSE_1_MASK_1	1
#define K_MOUSE_2_MASK_2	2
#define K_MOUSE_3_MASK_4	4
#define K_MOUSE_4_MASK_8	8
#define K_MOUSE_5_MASK_16	16

// Make a 2 value table.
// Quake name	Core Name	 // 
// K_INS		= K_INSERT	 // Do this to get the constants the same value assured.
							 // Remember the Quake constants names don't matter at all
							// Because K_INS isn't the same as the bind name "INS"
							// Same with K_DEL and "DEL"
// So one step could be to knock off keys.h
// And Core keyvalue_t, fuck that.  Should be what it says, key and value.
// Multiple table stuff, figure out some other name for it.

typedef enum {
	__UNUSED_K			= -1,   // Ensure MinGW makes us an int type and not an unsigned type
	K_INVALID_0			= 0,
	K_BACKSPACE         = 8,
	K_TAB               = 9,
	K_ENTER             = 13,	// Are we not able to detect the keypad enter?  Yes?  No?  Some operating systems yes/some no?  Grrr.
	K_ESCAPE            = 27,	// Remember that scan code is different from keymap.  It may keymap as enter, but it should scan code as kp_enter
	K_SPACE             = 32,	// Anyways ...
	K_APOSTROPHE        = 39,
	K_COMMA             = 44,
	K_MINUS             = 45,
	K_PERIOD            = 46,
	K_SLASH             = 47,
	K_SEMICOLON         = 59,

	K_EQUALS            = 61,
	K_LEFTBRACKET       = 91,
	K_BACKSLASH         = 92,
	K_RIGHTBRACKET      = 93,
	K_GRAVE             = 96,

	K_LCTRL             = 128,
	K_RCTRL             = 129,
	K_LALT              = 130,
	K_RALT              = 131,
	K_LSHIFT            = 132,
	K_RSHIFT            = 133,
	K_LWIN              = 134,
	K_RWIN              = 135,
	K_MENU              = 136,
	K_CAPSLOCK          = 137,
	K_NUMLOCK           = 138,
	K_SCROLLLOCK        = 139,
	K_PAUSE             = 140,

	// RESERVED: K_BREAK           = 141, ?  Or sysreq or who knows.

	K_PRINTSCREEN       = 142,
	K_INSERT            = 143,
	K_DELETE            = 144,
	K_LEFTARROW         = 145,
	K_RIGHTARROW        = 146,
	K_UPARROW           = 147,
	K_DOWNARROW         = 148,
	K_PAGEUP            = 149,
	K_PAGEDOWN          = 150,
	K_HOME              = 151,
	K_END               = 152,
	K_F1                = 153,
	K_F2                = 154,
	K_F3                = 155,
	K_F4                = 156,
	K_F5                = 157,
	K_F6                = 158,
	K_F7                = 159,
	K_F8                = 160,
	K_F9                = 161,
	K_F10               = 162,
	K_F11               = 163,
	K_F12               = 164,

	K_NUMPAD_0          = 177,		// BEGIN: These may do a keyboard emission ... (Numlock on presumably)
	K_NUMPAD_1          = 178,
	K_NUMPAD_2          = 179,
	K_NUMPAD_3          = 180,
	K_NUMPAD_4          = 181,
	K_NUMPAD_5          = 182,
	K_NUMPAD_6          = 183,
	K_NUMPAD_7          = 184,
	K_NUMPAD_8          = 185,
	K_NUMPAD_9          = 186,
	K_NUMPAD_MULTIPLY   = 187,
	K_NUMPAD_PLUS       = 188,
	K_NUMPAD_SEPARATOR  = 189,
	K_NUMPAD_MINUS      = 190,
	K_NUMPAD_PERIOD     = 191,
	K_NUMPAD_DIVIDE     = 192,		// END: These may do a keyboard emission 
	K_NUMPAD_ENTER      = 193,

// Plenty of extra space from 194 to 255 for future oddball keys

	K_MOUSE1            = 256,
	K_MOUSE2            = 257,
	K_MOUSE3            = 258,
	K_MOUSE4            = 259,
	K_MOUSE5            = 260,
// RESERVED: Possible Extra mouse buttons
	K_MOUSEWHEELUP      = 264,
	K_MOUSEWHEELDOWN    = 265,
// Future, right and up wheel?  A Mac touch pad simulates thoses.  Not sure, a Mac touch pad jumbles the ideas of dragging with scrolling.
// And we probably wouldn't treat that as a button --- there's no pressed or released state of a gesture.

//	K_MOUSEWHEELLEFT    = 266,
//	K_MOUSEWHEELRIGHT   = 267,
	K_JOY1              = 268,
	K_JOY2              = 269,
	K_JOY3              = 270,
	K_JOY4              = 271,
	K_AUX1              = 272,
	K_AUX2              = 273,
	K_AUX3              = 274,
	K_AUX4              = 275,
	K_AUX5              = 276,
	K_AUX6              = 277,
	K_AUX7              = 278,
	K_AUX8              = 279,
	K_AUX9              = 280,
	K_AUX10             = 281,
	K_AUX11             = 282,
	K_AUX12             = 283,
	K_AUX13             = 284,
	K_AUX14             = 285,
	K_AUX15             = 286,
	K_AUX16             = 287,
	K_AUX17             = 288,
	K_AUX18             = 289,
	K_AUX19             = 290,
	K_AUX20             = 291,
	K_AUX21             = 292,
	K_AUX22             = 293,
	K_AUX23             = 294,
	K_AUX24             = 295,
	K_AUX25             = 296,
	K_AUX26             = 297,
	K_AUX27             = 298,
	K_AUX28             = 299,
	K_AUX29             = 300,
	K_AUX30             = 301,
	K_AUX31             = 302,
	K_AUX32				= 303,
// Reserve a block starting at 384 for custom stuff?
} key_scancode_e;

typedef double ticktime_t;

///////////////////////////////////////////////////////////////////////////////
//  CORE: Basic function setup
///////////////////////////////////////////////////////////////////////////////

typedef void (*error_fn_t) (const char *error, ...); // __core_attribute__((__format__(__printf__,1,2), __noreturn__));
typedef int (*print_fn_t) (const char *fmt, ...); // __core_attribute__((__format__(__printf__,1,2)));

typedef FILE * (*fopenread_fn_t) (const char *, const char *);
typedef FILE * (*fopenwrite_fn_t) (const char *, const char *);
typedef int (*fclose_fn_t) (FILE*);

typedef void * (*malloc_fn_t) (size_t);
typedef void * (*calloc_fn_t) (size_t, size_t);
typedef void * (*realloc_fn_t)(void *, size_t);
typedef char * (*strdup_fn_t) (const char*);
typedef void (*free_fn_t)(void *);


typedef struct
{
	error_fn_t		ferror_fn;
	print_fn_t		fwarning_fn;
	print_fn_t		fprint_fn;
	print_fn_t		fdprint_fn;

	malloc_fn_t		fmalloc_fn;
	calloc_fn_t		fcalloc_fn;
	realloc_fn_t	frealloc_fn;
	strdup_fn_t		fstrdup_fn;

	free_fn_t		ffree_fn;

	fopenread_fn_t	ffopenread_fn;
	fopenwrite_fn_t	ffopenwrite_fn;
	fclose_fn_t		ffclose_fn;
} fn_set_t;


// Initializer, application passes function set.  appname is important
// and affects appdata folder names and window titles.

#include "image.h" // Moved to below function declares ...
#include "download.h" // Moved to below function declares ...

void Core_Init (const char *appname, fn_set_t *fnset, sys_handle_t handle );


///////////////////////////////////////////////////////////////////////////////
//  SYSTEM OS: MESSAGEBOX
///////////////////////////////////////////////////////////////////////////////

int System_MessageBox (const char *title, const char *fmt, ...) __core_attribute__((__format__(__printf__,2,3)));
int System_Alert (const char *fmt, ...); __core_attribute__((__format__(__printf__,1,2))) ;

///////////////////////////////////////////////////////////////////////////////
//  SYSTEM OS: PROCESSES
///////////////////////////////////////////////////////////////////////////////

sys_handle_t System_Process_Create (const char *path_to_file, const char *args, const char *working_directory_url);
int System_Process_Still_Running (sys_handle_t pid);
int System_Process_Terminate_Console_App (sys_handle_t pid);
int System_Process_Close (sys_handle_t pid);

#ifdef PLATFORM_WINDOWS
#define System_GL_GetProcAddress wglGetProcAddress
#else // not PLATFORM_WINDOWS ...
void * System_GetProcAddress (const char *pfunction_name);
#define System_GL_GetProcAddress System_GetProcAddress
#endif // !PLATFORM_WINDOWS

// #define FSOUND_GETFUNC(f, g) (qFSOUND_##f = (void *)GetProcAddress(fmod_handle, "_FSOUND_" #f #g))
// #define FSOUND_GETFUNC(f, g) (qFSOUND_##f = (void *)dlsym(fmod_handle, "FSOUND_" #f))




#ifdef CORE_LOCAL

///////////////////////////////////////////////////////////////////////////////
//  CORE: Private Local Shared
///////////////////////////////////////////////////////////////////////////////

char gCore_Appname[MAX_OSPATH];
sys_handle_t gCore_hInst; // Why is this a pointer?  Undid that.
sys_handle_t gCore_Window; // DO NOT LIKE

extern fopenread_fn_t	core_fopen_read;
extern fopenwrite_fn_t	core_fopen_write;
extern fclose_fn_t		core_fclose;

extern malloc_fn_t		core_malloc;
extern calloc_fn_t		core_calloc;
extern realloc_fn_t		core_realloc;
extern strdup_fn_t		core_strdup;
extern free_fn_t		_core_free;

extern error_fn_t		Core_Error; // Terminate
extern print_fn_t		Core_Warning; // Special notification
extern print_fn_t		Core_Printf; // Informational notification
extern print_fn_t		Core_DPrintf; // Secondary notification

void* core_free (const void* ptr); // Returns null

///////////////////////////////////////////////////////////////////////////////
//  SYSTEM OS: TIME
///////////////////////////////////////////////////////////////////////////////

void System_Sleep (unsigned long milliseconds);
double System_Time (void);
double System_Time_Now_Precise (void); // no set metric

///////////////////////////////////////////////////////////////////////////////
//  SYSTEM OS: URL QUERY
///////////////////////////////////////////////////////////////////////////////

const char * System_URL_Binary (void);
const char * System_URL_Binary_Folder (void);
const char * System_URL_Caches (void);

///////////////////////////////////////////////////////////////////////////////
//  SYSTEM OS: FILE DIALOGS PROMPT FOR A FILE OR FOLDER
///////////////////////////////////////////////////////////////////////////////

const char * System_Dialog_Open_Directory (const char *title, const char *starting_folder_url);
const char * System_Dialog_Open_Type (const char *title, const char * starting_folder_url, const char *extensions_comma_delimited);

// starting_file_url is default save name, add a "/" merely to suggest starting folder
const char * System_Dialog_Save_Type (const char *title, const char * starting_file_url, const char *extensions_comma_delimited);


///////////////////////////////////////////////////////////////////////////////
//  SYSTEM OS: FILE MANAGER INTERACTION
///////////////////////////////////////////////////////////////////////////////

cbool System_Folder_Open (const char *path_url);
cbool System_Folder_Open_Highlight (const char *path_to_file);
cbool System_Folder_Open_Highlight_Binary (void);

///////////////////////////////////////////////////////////////////////////////
//  SYSTEM OS: FILE AND DIRECTORY MANAGEMENT
///////////////////////////////////////////////////////////////////////////////

void System_chdir (const char *path_url); // change dir
const char *System_getcwd (void); // current working directory
void System_mkdir (const char *path_url); // make dir
int System_rmdir (const char *path_url); // remove dir
cbool System_File_Exists (const char *path_to_file); // file existence
cbool System_File_Is_Folder (const char *path_to_file);
size_t System_File_Length (const char *path_to_file);
double System_File_Time (const char *path_to_file);
cbool System_File_URL_Is_Relative (const char *path_to_file);

///////////////////////////////////////////////////////////////////////////////
//  SYSTEM OS: CLIPBOARD OPERATIONS
///////////////////////////////////////////////////////////////////////////////

#define SYS_CLIPBOARD_SIZE_256 256
const char *System_Clipboard_Get_Text_Line (void);
const char *System_Clipboard_Get_Text_Alloc (void);
cbool System_Clipboard_Set_Text (const char *text_to_clipboard);
cbool System_Clipboard_Set_Image_RGBA (const unsigned *rgba, int width, int height);
unsigned *System_Clipboard_Get_Image_RGBA_Alloc (int *outwidth, int *outheight);

#endif // CORE_LOCAL

#endif // ! __CORE_H__



