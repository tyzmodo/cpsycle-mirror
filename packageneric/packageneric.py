#! /usr/bin/env python

###################################################################
###################################################################
###################################################################

sp = SourcePackage(
	name = 'psycle',
	version = [0, 0, 0]
)

m = Module(
	name = 'lib-psycle.engine',
	type = Module.types.lib,
	version = [0, 0, 0]
)

m.requires.add(
	debian = ['lib-universalis-dev >= 0'],
	pkgconfig = ['universalis >= 0']
)

m.sources.add(find('src', 'psycle/engine', '*.cpp'))
m.headers.add(find('src', 'psycle/engine', '*.hpp'))

m.pkgconfig(
	name = 'psycle.engine',
	description = 'psycle engine library'
)

mp = ModulePackage(
	name = 'psycle.plugins',
	version = [0, 0, 0],
	description = 'psycle plugins'
)

mp.add([psycle.plugin.sine, psycle.plugin.output.default])

bp = binary_package(
	name = 'lib-hello',
	version = [0, 0, 0]
	description = 'hi!'
	long_description =
		"""
			hello, world!
		"""
)

bp.add(mp.modules)
bp.files.add(destination = os.path.join(bp.share, 'pixmaps', sp.name), find('pixmaps', '*.xpm'))

bp_dev = binary_package('lib-hello-dev', [mp], version = [0, 0, 0])
bp_dev.add([mp.headers, mp.pkgconfigs])

da = distribution_archive('shell.sourceforge.net:/home/groups/p/ps/psycle/htdocs/packages/debian')
da.add('sid', 'unstable', [sp, bp, bp_dev])

###################################################################
###################################################################
###################################################################

import os, fnmatch

class EnvList:
	def __init__(self):
		env = Environment()
		dictionary = env.Dictionary()
		keys = dictionary.keys()
		keys.sort()
		for key in keys:
			print '%s = %s' % (key, dictionary[key])

class Find:
	# a forward iterator that traverses a directory tree
	
	def __init__(self, directory, pattern="*"):
		self.stack = [directory]
		self.pattern = pattern
		self.files = []
		self.index = 0
	
	def __getitem__(self, index):
		while 1:
			try:
				file = self.files[self.index]
				self.index = self.index + 1
			except IndexError:
				# pop next directory from stack
				self.directory = self.stack.pop()
				self.files = os.listdir(self.directory)
				self.index = 0
			else:
				# got a filename
				fullname = os.path.join(self.directory, file)
				if os.path.isdir(fullname): # and not os.path.islink(fullname):
					self.stack.append(fullname)
				if fnmatch.fnmatch(file, self.pattern):
					return fullname

class CPPDefine:
	def __init(name, value = ''):
		seff.name = name
		self.value = value

class Source:
	def __init__(self, filename):
		self.filename = filename
		self.cppflags = ''

class Header:
	def __init__(self, filename):
		self.filename = filename
	
class Object:
	def __init__(self, source):
		self.source = source
	
	def scons(self):
		scons.Object(source)
	
class Module:
	def __init__(self, name, type):
		self.name = name
		self.type = type
		self.cppflags = []
		self.sources = []
		self.objects = []
		self.headers = []
		self.public_requires = []
		self.private_requires = []
	
	delf cppflags(self):
		return self.cppflags
	
	def sources(self):
		return self.sources
		
	def headers(self):
		return self.headers

	def public_requires(self, module):
		return self.public_requires

	def private_requires(self, module):
		return self.private_requires
	
	del merged_cppflags(self):
		result = self.cppflags
		for x in self.requires:
			result.append(x.merged_cppflags)
		return result

class ModulePackage
	def __init__(self, name):
		self.name = name
		self.cflags = []
		self.libs = []
		self.public_requires = []
		self.private_requires = []
	
	def cflags(self):
		return self.cflags
		
	def libs(self):
		return self.libs
	
	def public_requires(self):
		return self.public_requires
		
	def private_requires(self):
		return self.private_requires

	def merged_cppflags(self):
		result = self.cppflags
		for x in self.requires:
			result.append(x.merged_cppflags)
		return result

class BinaryPackage
	def __init__(self, name):
		self.name = name
		self.binary_version = 0
		self.package_version = 0

class DistributionArchive
	def __init__(self. remote_path):
		self.remote_path = remote_path
