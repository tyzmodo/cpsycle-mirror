// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2008 members of the psycle project http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\implementation universalis::cpu::exception
#include <universalis/detail/project.private.hpp>
#include "exception.hpp"
#include <universalis/os/loggers.hpp>
#include <universalis/os/thread_name.hpp>
#include <universalis/compiler/typenameof.hpp>
#include <universalis/stdlib/thread.hpp>
#if defined DIVERSALIS__OS__MICROSOFT
	#include <windows.h>
#endif
#if defined DIVERSALIS__COMPILER__MICROSOFT
	#include <eh.h>
#endif

#include "exceptions/code_description.hpp" // weird, must be included last or mingw 3.4.1 segfaults

namespace universalis { namespace cpu {

#if !defined NDEBUG
	exception::exception(unsigned int const & code, compiler::location const & location) throw()
	: os::exception(code, location)
	{
		// This type of exception is usually likely followed by an abrupt termination of the whole process.
		// So, we automatically log them as soon as they are created, that is, even before they are thrown.
		if(os::loggers::crash()()) {
			std::ostringstream s;
			s
				<< "cpu/os exception: "
				"thread: name: " << os::thread_name::get() << ", id: " << stdlib::this_thread::id() << '\n'
				<< compiler::typenameof(*this) << ": " << what();
			os::loggers::crash()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
	}
#endif

namespace {
	#if defined DIVERSALIS__OS__MICROSOFT
		void structured_exception_translator(unsigned int code, ::EXCEPTION_POINTERS * exception_pointers) throw(exception) {
			#if defined DIVERSALIS__STDLIB__RUNTIME__DEBUG
				#if !defined NDEBUG
					::EXCEPTION_RECORD const & exception_record(*exception_pointers->ExceptionRecord);
				#endif
				assert(code == exception_record.ExceptionCode);
			#endif
			switch(code) {
				////////////////////
				// things to ignore

				case STATUS_BREAKPOINT: // [bohan] not sure what to do with break points...
				case STATUS_SINGLE_STEP:
					return;

				/////////////////////////////
				// floating point exceptions

				case STATUS_FLOAT_INEXACT_RESULT:
				case STATUS_FLOAT_DENORMAL_OPERAND:
				case STATUS_FLOAT_UNDERFLOW:
					return; // unimportant exception, continue the execution.

				case STATUS_FLOAT_OVERFLOW:
				case STATUS_FLOAT_STACK_CHECK:
				case STATUS_FLOAT_DIVIDE_BY_ZERO:
				case STATUS_FLOAT_INVALID_OPERATION:
				default:
					throw exception(code, UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
			}

			///\todo add more information using ::EXCEPTION_POINTERS * exception_pointers
			#if 0
				// The GetModuleBase function retrieves the base address of the module that contains the specified address. 
				::DWORD static GetModuleBaseA(::DWORD dwAddress)
				{
					::MEMORY_BASIC_INFORMATION Buffer;
					return ::VirtualQuery((::LPCVOID) dwAddress, &Buffer, sizeof Buffer) ? (::DWORD) Buffer.AllocationBase : 0;
				}

				// Now print information about where the fault occured
				rprintf(_T(" at location %08x"), (::DWORD) pExceptionRecord->ExceptionAddress);
				if((hModule = (::HMODULE) ::GetModuleBaseA((::DWORD) pExceptionRecord->ExceptionAddress)) && ::GetModuleFileNameA(hModule, szModule, sizeof szModule))
					rprintf(_T(" in module %s"), szModule);
				
				// If the exception was an access violation, print out some additional information, to the error log and the debugger.
				if(pExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION && pExceptionRecord->NumberParameters >= 2)
					rprintf(" %s location %08x", pExceptionRecord->ExceptionInformation[0] ? "Writing to" : "Reading from", pExceptionRecord->ExceptionInformation[1]);
					
				/*drmingw::*/StackBackTrace(GetCurrentProcess(), GetCurrentThread(), pContext);
			#endif
		}
		
		static UNIVERSALIS__COMPILER__THREAD_LOCAL_STORAGE
		::LPTOP_LEVEL_EXCEPTION_FILTER thread_unhandled_exception_previous_filter(0);

		::LONG WINAPI unhandled_exception_filter(EXCEPTION_POINTERS * exception_pointers) throw(exception) {
			// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/debug/base/unhandledexceptionfilter.asp
			::EXCEPTION_RECORD const & exception_record(*exception_pointers->ExceptionRecord);
			unsigned int const code(exception_record.ExceptionCode);
			structured_exception_translator(code, exception_pointers);
			// note: The following code is never reached because structured_exception_translator throws a c++ exception.
			//       It is just kept here for documentation and because the function signature requires a return instruction.
			return thread_unhandled_exception_previous_filter ? thread_unhandled_exception_previous_filter(exception_pointers) : EXCEPTION_CONTINUE_SEARCH;
		}
	#endif
}

void exception::install_handler_in_thread() {
	if(os::loggers::trace()()) {
		std::ostringstream s;
		s << "installing cpu/os exception handler in thread: name: " << os::thread_name::get() << ", id: " << stdlib::this_thread::id();
		os::loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
	}
	#if defined DIVERSALIS__OS__MICROSOFT
		// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/debug/base/seterrormode.asp
		//::SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
			
		#if defined DIVERSALIS__COMPILER__MICROSOFT
			// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vclib/html/_crt__set_se_translator.asp
			// In a multithreaded environment,
			// translator functions are maintained separately for each thread.
			// Each new thread needs to install its own translator function.
			// Thus, each thread is in charge of its own translation handling.
			// There is no default translator function.
			// [bohan] This requires compilation with the asynchronous exception handling model (/EHa)
			::_set_se_translator(structured_exception_translator);
		#else
			static bool once(false);
			if(!once) {
				///\todo review and test this

				///\todo why only once?
				//once = true;
				// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/debug/base/setunhandledexceptionfilter.asp
				thread_unhandled_exception_previous_filter = ::SetUnhandledExceptionFilter(unhandled_exception_filter);

				#if defined DIVERSALIS__COMPILER__GNU
					// http://jrfonseca.dyndns.org/projects/gnu-win32/software/drmingw/index.html#exchndl
					// loads dr mingw's unhandled exception handler.
					// it does not matter if the library fails to load, we just won't have this intermediate handler.
					if(::LoadLibraryA("exchndl")) os::loggers::information()("unhandled exception filter loaded");
					else os::loggers::information()("unhandled exception filter has not been loaded");
				#endif
			}
		#endif
	#endif
}

}}
