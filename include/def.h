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
#define GLSLVIEW_STR_HELPER(x) #x
#define GLSLVIEW_STR(x) GLSLVIEW_STR_HELPER(x)
#define COMPILED_VERSION(major, minor, revision) GLSLVIEW_STR(major)GLSLVIEW_TEXT(".")GLSLVIEW_STR(minor)GLSLVIEW_TEXT(".")GLSLVIEW_STR(revision)




/*	Default value.	*/
#ifndef GLSLVIEW_MAJOR_VERSION
	#define GLSLVIEW_MAJOR_VERSION	0
#endif	/*	GLSLVIEW_MAJOR_VERSION	*/
#ifndef GLSLVIEW_MINOR_VERSION
	#define GLSLVIEW_MINOR_VERSION	5
#endif	/*	GLSLVIEW_MINOR_VERSION	*/
#ifndef GLSLVIEW_REVISION_VERSION
	#define GLSLVIEW_REVISION_VERSION 0
#endif	/*	GLSLVIEW_REVISION_VERSION	*/


#endif
