// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2009 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\interface psycle::host::plugin_resolver
#pragma once
#include <psycle/engine/engine.hpp>
#include <universalis/os/dyn_link.hpp>
#include <cassert>
#include <string>
#include <iostream>
#include <map>
#define PSYCLE__DECL  PSYCLE__HOST
#include <psycle/detail/decl.hpp>
namespace psycle { namespace host {

/// manages plugins.
/// resolves libraries, handles their loading and unloading,
/// instanciates nodes simply from their plugin names.
class PSYCLE__DECL plugin_resolver {
	private:
		class instanciator : protected engine::plugin_library_reference {
			friend class plugin_resolver;
			private:
				class instance {
					public:
						instance(instanciator &, engine::graph &, std::string const & name) throw(engine::exception);
						virtual ~instance() throw();
						inline operator engine::node const & () const throw() { return *node_; }
						inline operator engine::node       & ()       throw() { return *node_; }
						inline engine::node const & node() const throw() { return *node_; }
						inline engine::node       & node()       throw() { return *node_; }
					private:
						engine::node * const node_;
				};
			public:
				instanciator(plugin_resolver &, std::string const & name) throw(engine::exception);
				virtual ~instanciator() throw();
				instance & operator()(engine::graph &, std::string const & name);
				std::string               name() const throw();
				std::string          full_name() const throw();
				std::string const & short_name() const throw() { return plugin_library_reference::name(); }
			protected:
				unsigned int operator--() throw() /*override*/;
			private:
				plugin_resolver & plugin_resolver_;
				/// note: if aggregated directly without reference, on object destruction, we get "Inconsistency detected by ld.so: dl-close.c: 627: _dl_close: Assertion `map->l_init_called' failed!"
				universalis::os::dyn_link::resolver & library_resolver_;
				typedef engine::node & (* const node_instanciator) (engine::reference_counter &, engine::graph &, std::string const & name);
				node_instanciator const node_instanciator_;
		};
		instanciator & operator[](std::string const & plugin_name) throw(engine::exception);
		typedef std::map<std::string, instanciator*> map; map map_;
	public:
		/// creates a new plugin resolver.
		plugin_resolver();
		/// deletes the plugin resolver.
		/// To be able to delete the resolver correctly,
		/// there must be no more node instances of any of the plugins ;
		/// they must all have been deleted previous to this very object deletion.
		virtual ~plugin_resolver() throw();
		/// instanciates a node which is defined in the library plugin_name.
		/// The loading (and unloading) of the library is transparent.
		instanciator::instance & operator()(std::string const & plugin_name, engine::graph &, std::string const & node_name) throw(engine::exception);
};

}}
#include <psycle/detail/decl.hpp>
