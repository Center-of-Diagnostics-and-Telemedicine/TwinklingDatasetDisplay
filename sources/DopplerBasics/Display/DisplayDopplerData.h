#ifndef DopplerDataDisplay_h__
#define DisplayDopplerData_h__

/********************************************************************
	created:	2016/04/04
	created:	4:4:2016   16:03
	filename: 	q:\programs\DopplerTest\sources\DisplayDopplerData.h
	file path:	q:\programs\DopplerTest\sources
	file base:	DopplerDataDisplay
	file ext:	h
	author:		kns
	
	purpose:	
*********************************************************************/

#include <RFDataImport/S500_CFMFrameSet.h>


XRAD_BEGIN

void	DisplayDopplerData(const RealFunctionMD_F32 &cfm_phase_frames,
			const RealFunctionMD_F32 &cfm_phase_dispersion,
			const RealFunctionMD_F32 &cfm_phase_frames_normalized,
			const RealFunctionMD_F32 &in_mask_frames,
			const RealFunctionMD_F32 &correlation_frames,
			const RealFunctionMD_F32 &correlation_re_im_frames,
			const RealFunctionMD_F32 &std_conj_frames,
			const RealFunctionMD_F32 &amplitude_frames,
			const S500_CFMFrameSet &frames,
			const string &time_consumption);

XRAD_END

#endif // DopplerDataDisplay_h__