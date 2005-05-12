
// Note: this public configuration was not generated by an autoconf configuration script.

#pragma once
#define HAVE_CONFIG_H
#define OPERATING_SYSTEM__VERSION__MICROSOFT__COMPATIBILITY \
         OPERATING_SYSTEM__VERSION__MICROSOFT__COMPATIBILITY__1998



/// [bohan] added implementation of psycle::host::Song's lock using boost 1.3's read_write_mutex.
/// [bohan]
/// [bohan] I used a temporary #define (to be removed) to enable this new implementation of the gui<->audio thread synchronization.
/// [bohan] Once the new implementation is known to work well,
/// [bohan] we can remove this #define, which will trigger some #error in the places of the code that are concerned.
/// [bohan] Where the #error occurred, we can removed the old implementation.
/// [bohan]
/// [bohan] to enable this new implementation,
/// [bohan] #define PSYCLE__CONFIGURATION__OPTION__ENABLE__READ_WRITE_MUTEX 1
/// [bohan]
/// [bohan] to disable this new implementation, do not undefine the preprocessor symbol (which will triggers the #error's), but rather
/// [bohan] #define PSYCLE__CONFIGURATION__OPTION__ENABLE__READ_WRITE_MUTEX 0
#define PSYCLE__CONFIGURATION__OPTION__ENABLE__READ_WRITE_MUTEX 0



/// unmasks fpu exceptions
#define PSYCLE__CONFIGURATION__OPTION__ENABLE__FPU_EXCEPTIONS 1



/// JAZ: Define to enable the volume column for XMSampler.
///      It will also make the machine column in the pattern to show the values of the volume column instead.
// #define PSYCLE_OPTION_VOLUME_COLUMN

// value to show in the string describing the configuration options.
#if defined PSYCLE_OPTION_VOLUME_COLUMN
	#define PSYCLE__CONFIGURATION__OPTION__ENABLE__VOLUME_COLUMN 1
#else
	#define PSYCLE__CONFIGURATION__OPTION__ENABLE__VOLUME_COLUMN 0
#endif



/// the compiler used to build ... should be autodetermined, but we don't have autoconf.
#define PSYCLE__COMPILER__BUILD "msvc"



/// string describing the configuration options.
#define PSYCLE__CONFIGURATION__OPTIONS(EOL) \
	"compiler build tool chain = " PSYCLE__COMPILER__BUILD EOL \
	"read_write_mutex = " STRINGIZED(PSYCLE__CONFIGURATION__OPTION__ENABLE__READ_WRITE_MUTEX) EOL \
	"fpu exceptions = " STRINGIZED(PSYCLE__CONFIGURATION__OPTION__ENABLE__FPU_EXCEPTIONS) EOL \
	"volume column = " STRINGIZED(PSYCLE__CONFIGURATION__OPTION__ENABLE__VOLUME_COLUMN) EOL \
	"debugging = " PSYCLE__CONFIGURATION__OPTION__ENABLE__DEBUG
