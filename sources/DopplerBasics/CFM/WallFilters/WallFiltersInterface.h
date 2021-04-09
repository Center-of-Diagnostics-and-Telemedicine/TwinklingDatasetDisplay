#ifndef __wall_filter_interface
#define __wall_filter_interface


#include <DopplerBasics/CFM/WallFilters/WallFilters.h>

XRAD_BEGIN

enum wall_filter_type
{
	wf_average,
	//		wf_LS_interactive,
	wf_LS1,
	wf_LS2,
	wf_LS3,
	wf_f0,
	//wf_gaussian,
	wf_finite_difference,
//	wf_butterworth_recursive,
// 	wf_LEMD1,
// 	wf_LEMD2,
// 	wf_kl1_frame_average,
// 	wf_kl1_sweep_average,
// 	wf_kl1_beam_average,
// 	wf_kl2_frame_average,
// 	wf_kl2_sweep_average,
// 	wf_kl2_beam_average,

	wf_legendre1,
	wf_legendre2,
	wf_legendre3,
	wf_legendre4,

	wf_trigonometric1,
	wf_trigonometric2,
	wf_trigonometric3,
	wf_trigonometric4,

	no_wf,
	n_options
};


WallFilterPtr GetWallFilter(size_t burst_size, wall_filter_type option);

XRAD_END

#endif //__wall_filter_interface