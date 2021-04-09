#include "pre.h"

#include <XRADBasic/LinearVectorTypes.h>
#include <XRADBasic/MathMatrixTypes.h>
#include <XRADBasic/MathFunctionTypes2D.h>
#include <XRADBasic/FFT1D.h>

#include <RFDataImport/S500_CFMDataTypes.h>
#include <DopplerBasics/CFM/WallFilters/EMD.h>

#include <DopplerBasics/CFM/WallFilters/WallFiltersInterface.h>


XRAD_BEGIN


WallFilterPtr GetWallFilterInteractive(size_t burst_size)
{
	
	auto option = GetButtonDecision("Choose filter type",
	{
		MakeButton("Average", 	wf_average),
		MakeButton("F0 (??)", wf_f0),
		MakeButton("Finite difference ??", wf_finite_difference),

		MakeButton("Least squares, order 1", wf_LS1),
		MakeButton("Least squares, order 2", wf_LS2),
		MakeButton("Least squares, order 3", wf_LS3),
// 		MakeButton("Gaussian ??", wf_gaussian),
// 		MakeButton("Butterworth recursive ??", wf_butterworth_recursive),
		
// 		MakeButton("Linear EMD1", wf_LEMD1),
// 		MakeButton("Linear EMD2", wf_LEMD2),
// 
// 		MakeButton("KL1_frame average", wf_kl1_frame_average),
// 		MakeButton("KL1_sweep average", wf_kl1_sweep_average),
// 		MakeButton("KL1_beam average", wf_kl1_beam_average),
// 
// 		MakeButton("KL2_frame average", wf_kl2_frame_average),
// 		MakeButton("KL2_sweep average", wf_kl2_sweep_average),
// 		MakeButton("KL2_beam average", wf_kl2_beam_average),

		MakeButton("Legendre, order 1", wf_legendre1),
		MakeButton("Legendre, order 2", wf_legendre2),
		MakeButton("Legendre, order 3", wf_legendre3),
		MakeButton("Legendre, order 4", wf_legendre4),

		MakeButton("Trigonometric, order 1", wf_trigonometric1),
		MakeButton("Trigonometric, order 2", wf_trigonometric2),
		MakeButton("Trigonometric, order 3", wf_trigonometric3),
		MakeButton("Trigonometric, order 4", wf_trigonometric4),

		MakeButton("None", no_wf)


	});
	WallFilterPtr	wall_filter = GetWallFilter(burst_size, option);
	//	return WallFilterPtr(wall_filter);
	return wall_filter;

}

XRAD_END