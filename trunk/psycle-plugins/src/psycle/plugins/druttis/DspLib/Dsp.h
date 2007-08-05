//////////////////////////////////////////////////////////////////////
//
//	Dsp.h
//
//	druttis@darkface.pp.se
//
//////////////////////////////////////////////////////////////////////

#pragma pack ( )

//////////////////////////////////////////////////////////////////////
//	The class!
//////////////////////////////////////////////////////////////////////

class Dsp
{
	//////////////////////////////////////////////////////////////////
	//	Methods
	//////////////////////////////////////////////////////////////////

public:

	//////////////////////////////////////////////////////////////////
	//	Fill : a[0-n] = v
	//////////////////////////////////////////////////////////////////

	static inline void Fill(float *a, float v, int n)
	{
		--a;
		do {
			*++a = v;
		} while (--n);
	}

	//////////////////////////////////////////////////////////////////
	//	Addm : a[0-n] += b[0-n]
	//////////////////////////////////////////////////////////////////

	static inline void Addm(float *a, float *b, int n)
	{
		--a;
		--b;
		do {
			*++a += *++b;
		} while (--n);
	}

	//////////////////////////////////////////////////////////////////
	//	Subm : a[0-n] -= b[0-n]
	//////////////////////////////////////////////////////////////////

	static inline void Subm(float *a, float *b, int n)
	{
		--a;
		--b;
		do {
			*++a -= *++b;
		} while (--n);
	}

	//////////////////////////////////////////////////////////////////
	//	Mulm : a[0-n] *= b[0-n]
	//////////////////////////////////////////////////////////////////

	static inline void Mulm(float *a, float *b, int n)
	{
		--a;
		--b;
		do {
			*++a *= *++b;
		} while (--n);
	}

	//////////////////////////////////////////////////////////////////
	//	Addm : a[0-n] += v
	//////////////////////////////////////////////////////////////////

	static inline void Addm(float *a, float v, int n)
	{
		--a;
		do {
			*++a += v;
		} while (--n);
	}

	//////////////////////////////////////////////////////////////////
	//	Subm : a[0-n] -= v
	//////////////////////////////////////////////////////////////////

	static inline void Subm(float *a, float v, int n)
	{
		--a;
		do {
			*++a -= v;
		} while (--n);
	}

	//////////////////////////////////////////////////////////////////
	//	Mulm : a[0-n] *= v
	//////////////////////////////////////////////////////////////////

	static inline void Mulm(float *a, float v, int n)
	{
		--a;
		do {
			*++a *= v;
		} while (--n);
	}

	//////////////////////////////////////////////////////////////////
	//	Adds : a[0-n] += c[0-n], b[0-n] += c[0-n]
	//////////////////////////////////////////////////////////////////

	static inline void Adds(float *a, float *b, float *c, int n)
	{
		--a;
		--b;
		--c;
		do {
			*++a += *++c;
			*++b += *c;
		} while (--n);
	}

	//////////////////////////////////////////////////////////////////
	//	Subs : a[0-n] -= c[0-n], b[0-n] -= c[0-n]
	//////////////////////////////////////////////////////////////////

	static inline void Subs(float *a, float *b, float *c, int n)
	{
		--a;
		--b;
		--c;
		do {
			*++a -= *++c;
			*++b -= *c;
		} while (--n);
	}

	//////////////////////////////////////////////////////////////////
	//	Muls : a[0-n] *= c[0-n], b[0-n] *= c[0-n]
	//////////////////////////////////////////////////////////////////

	static inline void Muls(float *a, float *b, float *c, int n)
	{
		--a;
		--b;
		--c;
		do {
			*++a *= *++c;
			*++b *= *c;
		} while (--n);
	}

	//////////////////////////////////////////////////////////////////
	//	Adds : a[0-n] += v, b[0-n] += v
	//////////////////////////////////////////////////////////////////

	static inline void Adds(float *a, float *b, float v, int n)
	{
		--a;
		--b;
		do {
			*++a += v;
			*++b += v;
		} while (--n);
	}

	//////////////////////////////////////////////////////////////////
	//	Subs : a[0-n] -= v, b[0-n] -= v
	//////////////////////////////////////////////////////////////////

	static inline void Subs(float *a, float *b, float v, int n)
	{
		--a;
		--b;
		do {
			*++a -= v;
			*++b -= v;
		} while (--n);
	}

	//////////////////////////////////////////////////////////////////
	//	Muls : a[0-n] -= v, b[0-n] -= v
	//////////////////////////////////////////////////////////////////

	static inline void Muls(float *a, float *b, float v, int n)
	{
		--a;
		--b;
		do {
			*++a *= v;
			*++b *= v;
		} while (--n);
	}

};
