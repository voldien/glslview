/**
	glslview
    Copyright (C) 2016  Valdemar Lindberg

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/
#ifndef _GLSLVIEW_LOG_H_
#define _GLSLVIEW_LOG_H_ 1
#include"def.h"

/**
 *	Logging level.
 */
#define GLSLVIEW_QUITE		0x0		/*	Quite. Opresses all glslview_log_printf call.	*/
#define GLSLVIEW_VERBOSE	0x1		/*	Verbose, print only verbose.	*/
#define GLSLVIEW_DEBUG		0x3		/*	Debu, prints all.	*/

/**
 *	Set verbosity level.
 */
extern void glslview_set_verbosity_level(unsigned int verbosity);

/**
 *	Get verbosity level.
 */
extern unsigned int glslview_get_verbosity_level(void);

/**
 *	Print formated log level.
 *
 *	@Return number of bytes written.
 */
extern int glslview_log_printf(unsigned int verbosity, const char* fmt,...);

/**
 *	Verbose printf.
 */
#define glslview_verbose_printf(fmt, args...)	\
		glslview_log_printf(GLSLVIEW_VERBOSE, fmt, ##args)
/**
 *	Debug printf.
 */
#define glslview_debug_printf(fmt, args...)	\
		glslview_log_printf(GLSLVIEW_DEBUG, fmt, ##args)


#endif
