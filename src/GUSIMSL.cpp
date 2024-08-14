
#include "GUSIInternal.h"
#include "GUSIMSL.h"
#include "GUSIDescriptor.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <algorithm>

GUSI_USING_STD_NAMESPACE


extern __file_modes __temp_file_mode;

extern "C" {

FILE * __find_unopened_file();
// __handle_reopen is declared in <stdio.h>

void __flush_all(void);


int __open_file(const char * name, __file_modes mode, __file_handle * handle);

int __open_temp_file(__file_handle * handle);

int __read_file(
	__file_handle handle, unsigned char * buffer, size_t * count, __idle_proc idle_proc);

int __read_console(
	__file_handle handle, unsigned char * buffer, size_t * count, __idle_proc idle_proc);

int __write_file(
	__file_handle handle, unsigned char * buffer, size_t * count, __idle_proc idle_proc);

int __write_console(
	__file_handle handle, unsigned char * buffer, size_t * count, __idle_proc idle_proc);

int __position_file(
	__file_handle handle, unsigned long * position, int mode, __idle_proc idle_proc);

int __close_file(__file_handle handle);

int __close_console(__file_handle handle);

}

#warning: unhandled macro "definitions[mat]"
#warning: unhandled macro "definitions[mat]"

static void MSLSetupStdio(int fd, FILE * f)
{
	f->handle 			= fd;
	f->position_proc	= __position_file;
	f->read_proc 		= __read_file;
	f->write_proc 		= __write_file;
	f->close_proc 		= __close_file;
}

void GUSISetupConsoleStdio()
{
	MSLSetupStdio(0, stdin);
	MSLSetupStdio(1, stdout);
	MSLSetupStdio(2, stderr);
}

