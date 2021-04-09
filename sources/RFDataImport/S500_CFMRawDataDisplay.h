#ifndef __cfm_raw_data_display
#define __cfm_raw_data_display

#include <XRADGUI/Sources/GUI/MathFunctionGUIMD.h>
#include "S500_CFMFrameSet.h"

XRAD_BEGIN

//TODO объявленные две функции избыточны, возможно.
void	DisplayCFMFrameSet(const S500_CFMFrameSet &frames, wstring title);
//void	DisplayDuplex(const MathFunctionMD<ColorImageF32> &cfm_color_frames, const S500_CFMFrameSet &frames);
//void	DisplayDuplex(const MathFunctionMD<ColorImageF32> &cfm_color_frames, const S500_CFMFrameSet &frames);
//void	DisplayDuplex(const ColorImageMD_F32 &cfm_color_frames, const S500_CFMFrameSet &frames);

void	DisplayCFMDetailed(const cfm_container_t &cfm_frames, size_t n_cfm_shots, string title, const ScanConverterOptions &sco_cfm);


XRAD_END


#endif //__cfm_raw_data_display
