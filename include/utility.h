/**
    glslview
    Copyright (C) 2017  Valdemar Lindberg

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
#ifndef _GLSLVIEW_UTILITY_H_
#define _GLSLVIEW_UTILITY_H_ 1

/**
 *	Load file from filepath.
 *
 *	@Return if sucesfully the number of bytes loaded, -1 otherwise.
 */
extern long int glslview_loadfile(const char* cfilename, void** bufferptr);

/**
 *	Load string from filepath.
 *
 *	@Return if sucesfully the number of bytes loaded, -1 otherwise.
 */
extern long int glslview_loadString(const char* cfilename, void** bufferptr);

#endif
