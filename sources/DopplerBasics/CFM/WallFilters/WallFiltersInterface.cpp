#include "pre.h"

#include <XRADBasic/LinearVectorTypes.h>
#include <XRADBasic/MathMatrixTypes.h>
#include <XRADBasic/MathFunctionTypes2D.h>
#include <XRADBasic/FFT1D.h>

#include <RFDataImport/S500_CFMDataTypes.h>
#include <DopplerBasics/CFM/WallFilters/EMD.h>

#include <DopplerBasics/CFM/WallFilters/WallFiltersInterface.h>



XRAD_BEGIN

size_t	wall_filter_order(wall_filter_type option)
{
	switch(option)
	{
		case wf_f0: 
			return 1;

		case wf_trigonometric1:
		case wf_LS1:
		case wf_legendre1: 
			return 2;

		case wf_trigonometric2:
		case wf_LS2:
		case wf_legendre2: 
			return 3;

		case wf_trigonometric3:
		case wf_LS3:
		case wf_legendre3: 
			return 4;

		case wf_trigonometric4:
		case wf_legendre4:
			return 5;

		default:	throw invalid_argument("Unknown wall-filter basis");
	}

}

WallFilterPtr GetWallFilter(size_t burst_size, wall_filter_type option)
{
	WallFilter *wall_filter = NULL;
	switch (option)
	{

		case wf_finite_difference:
			wall_filter = new WallFilterFiniteDifference;
			break;



		case no_wf:
			wall_filter = new WallFilterNone;
			break;

		case wf_average:
			wall_filter = new WallFilterAverage;
			break;

		case wf_LS1:
		case wf_LS2:
		case wf_LS3:
		{
			WallFilterLS *wf = new WallFilterLS;
			wf->Init(burst_size, wall_filter_order(option));
			wall_filter = wf;
		}
		break;


		case wf_f0:
		case wf_trigonometric1:
		case wf_trigonometric2:
		case wf_trigonometric3:
		case wf_trigonometric4:
		case wf_legendre1:
		case wf_legendre2:
		case wf_legendre3:
		case wf_legendre4:
		{
			WallFilterBasis* wf = new WallFilterBasis;
			RealVectorF64	weights(burst_size, 1);
			std::fill(weights.begin(), weights.begin() + wall_filter_order(option), 0);

			switch(option)
			{
				case wf_f0:
				case wf_trigonometric1:
				case wf_trigonometric2:
				case wf_trigonometric3:
				case wf_trigonometric4:
					wf->InitTrigonometric(burst_size, weights);
					break;

				case wf_legendre1:
				case wf_legendre2:
				case wf_legendre3:
				case wf_legendre4:
					wf->InitLegendre(burst_size, weights);
					break;
			}
			wall_filter = wf;
		}
		break;



		default:
			throw invalid_argument("GetWallFilter, unknown filter type");
	}
	return WallFilterPtr(wall_filter);
};



XRAD_END