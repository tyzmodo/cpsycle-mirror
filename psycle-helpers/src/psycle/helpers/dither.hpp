// -*- mode:c++; indent-tabs-mode:t -*-
///\interface psycle::helpers::dsp::Dither
#include <psycle/helpers/mersennetwister.hpp>
namespace psycle { namespace helpers { namespace dsp {

class Dither {
	public:
		Dither();
		virtual ~Dither() {}

		void Process(float * inSamps, unsigned int length);

		enum Pdf {
			triangular = 0,
			rectangular,
			gaussian
		};
		
		enum NoiseShape {
			none = 0,
			highpass
		};

		void SetBitDepth(unsigned int newdepth) { bitdepth = newdepth; }
		void SetPdf(Pdf newpdf) { pdf = newpdf; }
		void SetNoiseShaping(NoiseShape newns) { noiseshape = newns; }

	private:
		unsigned int bitdepth;
		Pdf pdf;
		NoiseShape noiseshape;

		MersenneTwister mt;
};

}}}
