#include "pre.h"

#include <XRADBasic/Sources/Utils/ExponentialBlurAlgorithms.h> 
#include <RFDataImport/S500_CFMRawDataDisplay.h>
#include <DopplerBasics/Display/DuplexDisplayer.h>
#include <DopplerBasics/Display/DisplayDopplerData.h>

#include <DopplerBasics/Unknown/DopplerDataDisplayAdditional.h>

/********************************************************************
created:	2016/04/04
created:	4:4:2016   16:02
filename: 	q:\programs\DopplerTest\sources\DopplerDataDisplay.cpp
file path:	q:\programs\DopplerTest\sources
file base:	DopplerDataDisplay
file ext:	cpp
author:		kns

purpose:	
*********************************************************************/

XRAD_BEGIN

void	DisplayDopplerData(
	const RealFunctionMD_F32 &cfm_phase_frames,
	const RealFunctionMD_F32 &cfm_phase_dispersion,
	const RealFunctionMD_F32 &phase_frames_normalized,
	const RealFunctionMD_F32 &mask_frames,
	const RealFunctionMD_F32 &correlation_frames,
	const RealFunctionMD_F32 &correlation_re_im_frames,
	const RealFunctionMD_F32 &std_frames,
	const RealFunctionMD_F32 &amplitude_frames,
	const S500_CFMFrameSet	&frames,
	const string &time_consumption)
{
	DuplexDisplayer	dd(frames, phase_frames_normalized, mask_frames);

	RealFunctionMD_F32 std(std_frames);
	enum
	{
		display_duplex,
		display_phase,
		display_phase_dispersion,
		display_phase_normalized,
		display_mask,
		display_correlation,
		display_correlation_re_im,
		display_std,
		display_amplitude,
		display_additional,
		display_time_consumption,
		display_exit
	} option;
	while(true)
	{
		option = GetButtonDecision("Choose data to display",
		{
			MakeButton(L"Duplex",					display_duplex),
			MakeButton(L"Phase",					display_phase),
			MakeButton(L"Phase Dispersion",			display_phase_dispersion),
			MakeButton("Phase Normalized",			display_phase_normalized),
			MakeButton("Mask",						display_mask),
			MakeButton("Correlation map",			display_correlation),
			MakeButton("Correlation Re-Im map",		display_correlation_re_im),
			MakeButton("Standard deviation map",	display_std),
			MakeButton("Amplitude map",				display_amplitude),
			MakeButton("Additional",				display_additional),
			MakeButton("Time consumption",			display_time_consumption),
			
			MakeButton("Exit",						display_exit)
		});

		auto &sco = frames.sco_cfm;

		try
		{
			switch(option)
			{
				case display_duplex:
					dd.DisplayDuplex();
					break;

				case display_phase:
					DisplayMathFunction3D(cfm_phase_frames, "Phase", sco);
					break;

				case display_phase_dispersion:
					DisplayMathFunction3D(cfm_phase_dispersion, "Phase dispersion", sco);
					break;

				case display_phase_normalized:
					DisplayMathFunction3D(phase_frames_normalized, "Phase Normalized", sco);
					break;

				case display_mask:
					DisplayMathFunction3D(mask_frames, "Mask", sco);
					break;

				case display_correlation:
					DisplayMathFunction3D(correlation_frames, "Correlation", sco);
					break;

				case display_correlation_re_im:
					DisplayMathFunction3D(correlation_re_im_frames, "Correlation Re-Im", sco);
					break;

				case display_std:
					DisplayMathFunction3D(std_frames, "Amplitude std. deviation", sco);
					break;

				case display_amplitude:
					DisplayMathFunction3D(amplitude_frames, "Amplitude", sco);
					break;

				case display_additional:
					DisplayDopplerDataAdditional(correlation_frames, std_frames, amplitude_frames, frames);
					break;

				case display_time_consumption:
					ShowText("Time consumption report", time_consumption);
					break;

				default:
					throw canceled_operation("");
			}
		}
		catch(canceled_operation &) { throw; }
		catch (quit_application &) { throw; }
		catch (...) { Error(GetExceptionString()); }
	}
}




XRAD_END