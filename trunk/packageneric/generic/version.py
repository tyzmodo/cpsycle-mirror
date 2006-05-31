class version:
	def __init__(
		self,
		major = 0,
		minor = 0,
		patch = 0
	):
		self._major = major
		self._minor = minor
		self._patch = patch

	def major(self):
		return self._major
		
	def minor(self):
		return self._minor
		
	def patch(self):
		return self._patch
		
	def __str__(self):
		return str(self._major) + '.' + str(self._minor) + '.' + str(self._patch)
	
	def __cmp__(self, other):
		result = cmp(self.major(), other.major())
		if result:
			return result
		result = cmp(self.minor(), other.minor())
		if result:
			return result
		return cmp(self.patch(), other.patch())
