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
#ifndef _DEF_H_
#define _DEF_H_ 1


/**
 *    Compiler.
 */
#ifdef _MSC_VER 	/*	Visual Studio C++ Compiler.	*/
	#define GLSLVIEW_VC
	#define GLSLVIEW_COMPILER 1
	#if _MSC_VER >= 1900
		#define GLSLVIEW_V13 _MSC_VER
	#elif _MSC_VER >= 1800
		#define GLSLVIEW_V12 _MSC_VER
	#elif _MSC_VER >= 1700
		#define GLSLVIEW_VC11 _MSC_VER
	#elif _MSC_VER >= 1600
		#define GLSLVIEW_VC10 _MSC_VER
	#elif _MSC_VER >= 1500
		#define GLSLVIEW_VC9 _MSC_VER
	#elif _MSC_VER >= 1400
		#define GLSLVIEW_VC8 _MSC_VER
	#elif _MSC_VER >= 1300
		#define GLSLVIEW_VC7 _MSC_VER
	#else
		#define GLSLVIEW_VC6 _MSC_VER
	#endif
    	#pragma warning(disable : 4201)
	#define GLSLVIEW_COMPILER_NAME "Visual Studio C++/C"

#elif defined(__clang__)  || defined(__llvm__)           /*  LLVM, clang   */
    #define GLSLVIEW_LLVM 1
	#define GLSLVIEW_CLANG 1
	#define GLSLVIEW_COMPILER 5
	#define GLSLVIEW_COMPILER_NAME "LLVM/CLANG"
	#define GLSLVIEW_COMPILER_MAJOR_VERSION __clang_major__
	#define GLSLVIEW_COMPILER_MINOR_VERSION __clang_minor__

#elif defined(__GNUC__) || defined(__SNC__) || defined( __GNUC_MINOR__)	/*  GNU C Compiler*/
	#define GLSLVIEW_GNUC 1
	#define GLSLVIEW_COMPILER 2
	#define GLSLVIEW_COMPILER_NAME "GNU C"
	#define GLSLVIEW_COMPILER_MAJOR_VERSION __clang_major__
	#define GLSLVIEW_COMPILER_MINOR_VERSION __clang_minor__

#elif defined(__GNUG__) /*  GNU C++ Compiler*/
	#define GLSLVIEW_GNUC 2

#elif defined(__ghs__)		/* GHS	*/
	#define GLSLVIEW_GHS 1
	#define GLSLVIEW_COMPILER 3

#elif defined(__HP_cc) || defined(__HP_aCC)			/*	*/

#elif defined(__PGI)			/*	*/

#elif defined(__ICC) || defined(__INTEL_COMPILER) /*  Intel Compiler  */
	#define GLSLVIEW_INTEL
	#define GLSLVIEW_COMPILER 4
	#define GLSLVIEW_COMPILER_NAME "Intel C++"

#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)

#else
	#error Unsupported Compiler.
#endif


/**
 *	Platform define
 *	Architecture!
 */
#ifdef GLSLVIEW_VC
	#if defined(_M_IX86) || defined(_WIN32)
		#define GLSLVIEW_X86                          /**/
		#define GLSLVIEW_X32                          /**/
		#define GLSLVIEW_WIN32                        /**/
		#define GLSLVIEW_WINDOWS                      /**/
		#define GLSLVIEW_ARCH "x86"
	#elif defined(_M_X64) || defined(_WIN64)
		#define GLSLVIEW_X64                          /**/
		#define GLSLVIEW_WIN64                        /**/
		#define GLSLVIEW_WINDOWS                      /**/
		#define GLSLVIEW_ARCH "x64"
	#elif defined(_M_PPC)
		#define GLSLVIEW_PPC                          /**/
		#define GLSLVIEW_X360                         /**/
		#define GLSLVIEW_VMX                          /**/
	#elif defined(_M_ARM)
		#define GLSLVIEW_ARM                          /**/
		#define GLSLVIEW_ARCH "arm"
		#define GLSLVIEW_ARM_NEON                     /**/
	#endif
#elif defined(GLSLVIEW_GNUC) || defined(GLSLVIEW_CLANG)
	#ifdef __CELLOS_LV2__   /**/
        #define GLSLVIEW_PS3                          /*	playstation 3*/
	#elif defined(__arm__)	/**/
		#define GLSLVIEW_ARM
        #define GLSLVIEW_PSP2                         /*	playstation portable 2*/
        #define GLSLVIEW_RAS_PI                       /*	rasberry pi	*/
	#endif
	#if defined(_WIN32) /**  Window*/
		#define GLSLVIEW_X86
		#define GLSLVIEW_WINDOWS                      /**/
	#endif
	#if ( defined(__linux__) || defined(__linux) || defined(linux) ) && (!(__ANDROID__) || !(ANDROID))	/* Linux */
		#define GLSLVIEW_LINUX 1                       /**/
		#if defined(__amd64) || defined(__x86_64__) || defined(__i386__)
            #define GLSLVIEW_X86 1
			#define GLSLVIEW_ARCH "x86_64"
		#endif
		#if defined(__arm__)
              #define EX_ARM 1
        #endif

	#elif defined (ANDROID) || defined(__ANDROID__) || __ANDROID_API__ > 9  /** Android */
        #include<jni.h>
		#define GLSLVIEW_ANDROID 1
		/*  android Architecture*/
        #if defined(__arm__)
			#define GLSLVIEW_ARM 1
		  #if defined(__ARM_ARCH_7A__)
			#if defined(__ARM_NEON__)
			  #if defined(__ARM_PCS_VFP)
				#define GLSLVIEW_ABI "armeabi-v7a/NEON (hard-float)"
			  #else
				#define GLSLVIEW_ABI "armeabi-v7a/NEON"
			  #endif
			#else
			  #if defined(__ARM_PCS_VFP)
				#define GLSLVIEW_ABI "armeabi-v7a (hard-float)"
			  #else
				#define GLSLVIEW_ABI "armeabi-v7a"
			  #endif
			#endif
		  #else
		   #define GLSLVIEW_ABI "armeabi"
		  #endif
		#elif defined(__i386__)
		   #define GLSLVIEW_ABI "x86"
		#elif defined(__x86_64__)
		   #define GLSLVIEW_ABI "x86_64"
		#elif defined(__mips64)  /* mips64el-* toolchain defines __mips__ too */
		   #define GLSLVIEW_ABI "mips64"
		#elif defined(__mips__)
		   #define GLSLVIEW_ABI "mips"
		#elif defined(__aarch64__)
		   #define GLSLVIEW_ABI "arm64-v8a"
		#else
		   #define GLSLVIEW_ABI "unknown"
		#endif

	#elif defined (__APPLE__)   /*  Apple product   */
		#define GLSLVIEW_APPLE 1
		#if defined(__arm__)
			#define GLSLVIEW_APPLE_IOS    /*  Apple iphone/ipad OS    */
		#elif defined(MACOSX) || defined(macintosh) || defined(Macintosh)
			#define EX_MAC 1
		#endif
	#elif defined(__CYGWIN) 	/**/
		#define GLSLVIEW_CYGWIN 1
		#define GLSLVIEW_LINUX 1
	#elif defined(__FreeBSD__) || defined(__FreeBSD_kernel__)   /*  BSD*/
		#define GLSLVIEW_BSD
    	#elif defined(__llvm__) || defined(__clang__)   	/*  llvm    */
        	#define GLSLVIEW_LLVM 1
	#endif

#elif defined(__ICC) || defined(__INTEL_COMPILER)


#else
	#error  Unsupported architecture!   /*  No architecture support implicitly. remove this line to compile anyway*/
#endif

/**
 *	Check if UNIX platform.
 */
#if defined(__unix__) || defined(__unix) || defined(unix)	/*  Unix    */
	#   define GLSLVIEW_UNIX 1
#endif



/**
 *	Calling function convention.
 */
#ifdef GLSLVIEW_WINDOWS	        /** Windows Calling Convention.*/
	#define GLSLVIEWAPIENTRY     __cdecl
	#define GLSLVIEWAPIFASTENTRY __fastcall
	#define GLSLVIEWAPITHISENTRY __thiscall
	#define GLSLVIEWAPISTDENTRY  __stdcall
#elif defined(GLSLVIEW_ANDROID)   /** Android Calling Convention	*/
    #define GLSLVIEWAPIENTRY JNICALL
    #define GLSLVIEWAPIFASTENTRY JNICALL
    #define GLSLVIEWAPITHISENTRY JNICALL
    #define GLSLVIEWAPISTDENTRY JNICALL
#else
#   if !defined(__cdecl) && ( defined(GLSLVIEW_GNUC)  || defined(GLSLVIEW_CLANG) )
        #define __cdecl  __attribute__ ((__cdecl__))
        #define __stdcall  __attribute__ ((stdcall))
		#define __fastcall __attribute__((fastcall))
#   endif
	#define GLSLVIEWAPIENTRY     __cdecl
	#define GLSLVIEWAPISTDENTRY  __stdcall
	#define GLSLVIEWAPIFASTENTRY __fastcall
#endif


/**
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
