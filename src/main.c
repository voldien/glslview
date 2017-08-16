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
#include"glslview.h"
#include <unistd.h>

int main(int argc, const char** argv){

	int status = EXIT_SUCCESS;		/*	Exit status.	*/

	/*	Check if STDIN is piped.	*/
	g_isPipe = isatty(STDIN_FILENO) == 0;
	if(argc <= 1 && !g_isPipe){
		fprintf(stderr, "No arguments.\n");
		return EXIT_FAILURE;
	}

	/*	Initialize glslview.	*/
	if(glslview_init(argc, argv) == 0){
		fprintf(stderr, "glslview_init failed.\n");
		status = EXIT_FAILURE;
		goto error;
	}

	/*	Main display function.	*/
	if(!glslview_display()){
		fprintf(stderr, "glslview_display failed.\n");
		status = EXIT_FAILURE;
		goto error;
	}

	error:	/*	*/

	/*	Release resources.	*/
	glslview_verbose_printf("glslview is terminating.\n");
	glslview_gl_release();
	glslview_terminate();

	return status;
}
