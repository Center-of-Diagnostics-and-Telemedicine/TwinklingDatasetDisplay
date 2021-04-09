#ifndef __frame_set
#define __frame_set

#include <XRADBasic/MathFunctionTypes2D.h>
#include <XRADBasic/MathFunctionTypesMD.h>
#include <XRADSystem/CFile.h>
#include <XRADBasic/Sources/ScanConverter/ScanConverterOptions.h>

#include "S500_CFMParamFileData.h"
#include "S500_CFMFrameSetParams.h"
#include "S500_CFMDataTypes.h"


XRAD_BEGIN


class	S500_CFMFrameSet : public S500_CFMFrameSetParams
{
	public:
		PARENT(S500_CFMFrameSetParams);
	private:
		const S500_CFMParamFileData file_params;
	public:
		const size_t	start_frame;
		
	public:
		const size_t	n_frames;
		const size_t	frame_size;
		const size_t	n_b_rays;
		const size_t	n_b_samples;
				
		const size_t	cfm_density;


		const ptrdiff_t	first_cfm_ray;
		const ptrdiff_t	first_cfm_sample;
		const size_t	cfm_filter_order;

		
		// данные b
		b_container_t	b_frames;
		// данные доплера, разложенные по кадрам (все выстрелы вместе)
		cfm_container_t	cfm_frames;
		// данные доплера, разложенные по выстрелам
		cfm_container_t	cfm_shot_frames;
		
		
		ScanConverterOptions	sco_b, sco_cfm;

		S500_CFMFrameSet(const S500_CFMParamFileData &in_params, size_t in_start_frame, size_t in_end_frame);
		
		void	ReadFrame(shared_cfile file, size_t frame_no, bool unsweep);
		void	ReadAllFrames(bool unsweep);

};


XRAD_END


#endif //__frame_set
