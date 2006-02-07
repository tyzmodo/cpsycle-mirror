// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\file
///\interface universalis::compiler::detail::demangled
#pragma once
#include <universalis/detail/project.hpp>
#include <typeinfo>
#include <string>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK UNIVERSALIS__COMPILER__DETAIL__PRAGMATIC__DEMANGLE
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace universalis
{
	namespace compiler
	{
		///\internal
		namespace detail
		{
			///\internal
			/// demangling of compiler symbols strings.
			std::string UNIVERSALIS__COMPILER__DYNAMIC_LINK demangled(std::string const & mangled_symbol);
		}
	}
}
#include <universalis/compiler/dynamic_link/end.hpp>
