// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\interface psycle::plugins::sequence
#pragma once
#include <psycle/engine.hpp>
#include <psycle/host.hpp>
#include <psycle/plugins/sequence.hpp>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__TESTS__RANDOM_NOTES
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle { namespace tests { namespace random_notes {

class score1 {
	public:
		score1(host::plugin_resolver &, engine::graph &, engine::node & out);
		void connect();
		void generate();
	private:
		host::plugin_resolver & resolver_;
		engine::graph & graph_;
		engine::node & out_;

		engine::node & additioner_;
		engine::node & sine1_, & sine2_, & sine3_;
		plugins::sequence & freq1_, & freq2_, & freq3_;
		engine::node & decay1_, & decay2_, & decay3_;
		plugins::sequence & seq1_, & seq2_, & seq3_;
		plugins::sequence & decay_seq1_, & decay_seq2_, & decay_seq3_;
};

}}}
#include <universalis/compiler/dynamic_link/end.hpp>
