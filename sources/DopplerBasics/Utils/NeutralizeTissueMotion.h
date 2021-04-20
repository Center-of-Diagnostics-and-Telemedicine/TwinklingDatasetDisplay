#ifndef __analyzer
#define __analyzer

//#include "WallFilters.h"
//#include "KarhunenLoeve.h"
#include <DopplerBasics/CFDataPhaseAnalyzer.h>

XRAD_BEGIN

void	NeutralizeTissueMotionOneFrame(CFDataPhaseAnalyzer::shots_array_t  &slice, size_t n_beams_in_sweep, size_t n_shots, double accumulator_blur, bool acceleration_flag);

XRAD_END

#endif //__analyzer
