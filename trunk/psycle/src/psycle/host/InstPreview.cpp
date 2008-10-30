//\file
//\brief implementation file for psycle::host::InstPreview.
#include <psycle/project.private.hpp>
#include "InstPreview.hpp"
#include "Instrument.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

		void InstPreview::Work(float *pInSamplesL, float *pInSamplesR, int numSamples)
		{
			if(!m_pInstrument) return;

			float *pSamplesL = pInSamplesL;
			float *pSamplesR = pInSamplesR;
			--pSamplesL;
			--pSamplesR;
			
			short *wl = m_pInstrument->waveDataL;
			short *wr = m_pInstrument->waveDataR;
			float ld = 0;
			float rd = 0;
			
			do
			{
				ld=(*(wl+m_pos))*m_vol;
				if(m_pInstrument->waveStereo)
					rd=(*(wr+m_pos))*m_vol;
				else
					rd=ld;
					
				*(++pSamplesL)+=ld;
				*(++pSamplesR)+=rd;
					
				if(++m_pos>=m_pInstrument->waveLength)
				{
					Stop();
					return;
				}
				if(m_bLoop && m_pInstrument->waveLoopEnd == m_pos)
				{
					m_pos=m_pInstrument->waveLoopStart;
				}
				
			} while(--numSamples);
		}

		void InstPreview::Play(unsigned long startPos/*=0*/)
		{
			if(!m_pInstrument) return;

			m_bLoop = m_pInstrument->waveLoopType;
			if(startPos < m_pInstrument->waveLength)
			{
				m_bPlaying=true;
				m_pos=startPos;
			}
		}

		void InstPreview::Stop()
		{
			m_bPlaying=false;
		}
		
		void InstPreview::Release()
		{
			m_bLoop = false;
		}

	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
