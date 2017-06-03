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

#include "internal.h"
#include <unistd.h>

int main(int argc, const char** argv){

	int status = EXIT_SUCCESS;		/*	Exit status.	*/

	/*	Check if STDIN is piped.	*/
	isPipe = isatty(STDIN_FILENO) == 0;
	if(argc <= 1 && !isPipe){
		fprintf(stderr, "No arguments.\n");
		return EXIT_FAILURE;
	}

	/*	Initialize glslview.	*/
	if(glslview_init(argc, argv) == 0){
		status = EXIT_FAILURE;
		goto error;
	}

	/*	Main function.	*/
	glslview_display();

	error:	/*	*/

	/*	Release resources.	*/
	glslview_verbose_printf("glslview is terminating.\n");
	glslview_release_renderingapi();
	glslview_terminate();

	return status;
}
