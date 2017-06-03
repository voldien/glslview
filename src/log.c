#include"glslview_log.h"
#include"internal.h"
#include<stdarg.h>


void glslview_set_verbosity_level(unsigned int verbosity){
	verbose = verbosity;
}

unsigned int glslview_get_verbosity_level(void){
	return verbose;
}

int glslview_log_printf(unsigned int verbosity, const char* fmt,...){

	if(glslview_get_verbosity_level() <= verbosity){
		va_list ap;
		int res;

		va_start(ap, fmt);
		res = vprintf(fmt, ap);
		va_end(ap);

		return res;
	}

	return 0;
}
