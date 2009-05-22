#! /usr/bin/env python
# This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
# copyright 2006-2009 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

from wonderbuild.cxx_tool_chain import MultiBuildCheckTask, BuildCheckTask

class StdMathCheckTask(MultiBuildCheckTask):
	def __init__(self, base_cfg): MultiBuildCheckTask.__init__(self, 'c++-std-math', base_cfg)
		
	def do_check_and_set_result(self, sched_ctx):
		t = StdMathCheckTask.SubCheckTask(self, False)
		sched_ctx.parallel_wait(t)
		if t.result: self.results = t.result, t.m
		else:
			t = StdMathCheckTask.SubCheckTask(self, True)
			sched_ctx.parallel_wait(t)
			if t.result: self.results = t.result, t.m
			else: self.results = False, None
	
	@property
	def result(self): return self.results[0]
	
	@property
	def m(self): return self.results[1]

	@property
	def result_display(self):
		if self.result: return 'yes with' + (not self.m and 'out' or '') + ' lm', '32'
		else: return 'no', '31'
		
	def apply_to(self, cfg):
		if self.m: cfg.libs.append('m')

	@property
	def source_text(self): return \
		'#include <cmath>\n' \
		'double math() {\n' \
		'	float  const f(std::sin(1.f));\n' \
		'	double const d(std::sin(1. ));\n' \
		'	return d + f;\n' \
		'}'
		
	class SubCheckTask(BuildCheckTask):
		def __init__(self, outer, m):
			BuildCheckTask.__init__(self, outer.name + '-with' + (not m and 'out' or '') + '-lm', outer.base_cfg)
			self.outer = outer
			self.m = m

		def apply_to(self, cfg):
			if self.m: cfg.libs.append('m')

		@property
		def source_text(self): return self.outer.source_text
