///\file
///\brief interface file for psycle::host::Filter.
#pragma once
#include <cmath>
namespace psycle
{
	namespace host
	{
	namespace dsp
	{
		#define TPI 6.28318530717958647692528676655901

		enum FilterType{
			F_LOWPASS12 = 0,
			F_HIGHPASS12 = 1,
			F_BANDPASS12 = 2,
			F_BANDREJECT12 = 3,
			F_NONE = 4
		};

		class FilterCoeff
		{
		public:
			float _coeffs[5][128][128][5];
			FilterCoeff() { _inited = false; };
			inline void Init(void)
			{
				if (!_inited)
				{
					_inited = true;
					for (int r=0; r<5; r++)
					{
						for (int f=0; f<128; f++)
						{
							for (int q=0; q<128; q++)
							{
								ComputeCoeffs(f, q, r);
								_coeffs[r][f][q][0] = (float)_coeff[0];
								_coeffs[r][f][q][1] = (float)_coeff[1];
								_coeffs[r][f][q][2] = (float)_coeff[2];
								_coeffs[r][f][q][3] = (float)_coeff[3];
								_coeffs[r][f][q][4] = (float)_coeff[4];
							}
						}
					}
				}
			};
		private:
			bool _inited;
			double _coeff[5];
			void ComputeCoeffs(int freq, int r, int t);

			static inline float Cutoff(int v)
			{
				return float(pow( (v+5)/(127.0+5), 1.7)*13000+30);
			};
			
			static inline float Resonance(float v)
			{
				return float(pow( v/127.0, 4)*150+0.1);
			};
			
			static inline float Bandwidth(int v)
			{
				return float(pow( v/127.0, 4)*4+0.1);
			};
		};

		/// filter.
		class Filter
		{
		public:
			FilterType _type;
			int _cutoff;
			int _q;

			Filter();

			void Init(void);
			void Update(void);
			inline float Work(float x)
			{
				float y = _coeff0*x + _coeff1*_x1 + _coeff2*_x2 + _coeff3*_y1 + _coeff4*_y2;
				_y2 = _y1;
				_y1 = y;
				_x2 = _x1;
				_x1 = x;
				return y;
			};
			inline void WorkStereo(float& l, float& r)
			{
				float y = _coeff0*l + _coeff1*_x1 + _coeff2*_x2 + _coeff3*_y1 + _coeff4*_y2;
				_y2 = _y1;
				_y1 = y;
				_x2 = _x1;
				_x1 = l;
				l = y;
				float b = _coeff0*r + _coeff1*_a1 + _coeff2*_a2 + _coeff3*_b1 + _coeff4*_b2;
				_b2 = _b1;
				_b1 = b;
				_a2 = _a1;
				_a1 = r;
				r = b;
			};
		protected:
			static FilterCoeff _coeffs;
			float _coeff0;
			float _coeff1;
			float _coeff2;
			float _coeff3;
			float _coeff4;
			float _x1, _x2, _y1, _y2;
			float _a1, _a2, _b1, _b2;
		};


		class ITFilter
		{
		#define LOG10 2.30258509299 // neperian log10
		public:
			ITFilter()
				:  iSampleRate(44100)
			{
				Reset();
			};
			virtual ~ITFilter(){};
			void Reset(void)
			{
				ftFilter= F_NONE;
				iCutoff=127;
				iRes=0;
				fLastSampleLeft[0]=0.0f;
				fLastSampleLeft[1]=0.0f;
				fLastSampleRight[0]=0.0f;
				fLastSampleRight[1]=0.0f;
				Update();
			};
			void Cutoff(int _iCutoff) { if ( _iCutoff != iCutoff) { iCutoff = _iCutoff; Update(); }};
			void Ressonance(int _iRes) { if ( _iRes != iRes ) { iRes = _iRes; Update(); }};
			void SampleRate(int _iSampleRate) { if ( _iSampleRate != iSampleRate) {iSampleRate = _iSampleRate; Update(); }};
			void Type (FilterType newftype) { if ( newftype != ftFilter ) { ftFilter = newftype; Update(); }};
			FilterType Type (void) { return ftFilter; };
			
			inline void Work(float& _fSample)
			{
				const float fy = (_fSample * fCoeff[0]) + (fLastSampleLeft[1] * fCoeff[1]) + (fLastSampleLeft[0] * fCoeff[2]);
				fLastSampleLeft[0] = fLastSampleLeft[1];
				fLastSampleLeft[1] = fy - (_fSample * fCoeff[3]);
				_fSample = fy;
			}
			inline void WorkStereo(float& _fLeft, float& _fRight)
			{
				const float fyL = (_fLeft * fCoeff[0]) + (fLastSampleLeft[1] * fCoeff[1]) + (fLastSampleLeft[0] * fCoeff[2]);
				fLastSampleLeft[0] = fLastSampleLeft[1];
				fLastSampleLeft[1] = fyL - (_fLeft * fCoeff[3]);
				_fLeft = fyL;

				const float fyR = (_fRight * fCoeff[0]) + (fLastSampleRight[1] * fCoeff[1]) + (fLastSampleRight[0] * fCoeff[2]);
				fLastSampleRight[0] = fLastSampleRight[1];
				fLastSampleRight[1] = fyR - (_fRight * fCoeff[3]);
				_fRight = fyR;
			}
		protected:
			void Update(void);

			int iSampleRate;
			int iCutoff;
			int iRes;
			FilterType ftFilter;
			float fCoeff[4];
			float fLastSampleLeft[2];
			float fLastSampleRight[2];
		};
	}
	}
}
