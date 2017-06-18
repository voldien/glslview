#ifndef _DEF_H_
#define _DEF_H_ 1


/*
 *	Unicode macro for converting constant string to
 */
#ifdef UNICODE  /*  UTF-16*/
	#define GLSLVIEW_TEXT(quote) L##quote
	#define GLSLVIEW_TEXT(quote)  _EX_TEXT(quote)
#else           /*	ASCII / UTF-8	*/
    #define GLSLVIEW_TEXT(quote) quote
#endif

/**
 *	Macro string.
 */
#define GLSLVIEW_STR_HELPER(x) #x
#define GLSLVIEW_STR(x) GLSLVIEW_STR_HELPER(x)
#define COMPILED_VERSION(major, minor, revision, state) GLSLVIEW_STR(major)GLSLVIEW_TEXT(".")GLSLVIEW_STR(minor)GLSLVIEW_STR(state)GLSLVIEW_STR(revision)

#endif
