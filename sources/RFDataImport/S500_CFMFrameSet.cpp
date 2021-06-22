#include "pre.h"
#include <XRADBasic/MathFunctionTypesMD.h>
#include <XRADGUI/Sources/Gui/MathFunctionGUI2D.h>
#include <XRADGUI/Sources/Gui/MathFunctionGUIMD.h>
#include "S500_CFMFrameSet.h"
#include <omp.h>

XRAD_BEGIN



S500_CFMFrameSet::S500_CFMFrameSet( const S500_CFMParamFileData &in_params, size_t in_start_frame, size_t in_end_frame ) :
		parent(in_params),
		file_params(in_params),
		start_frame(in_start_frame),
		n_frames(range(in_end_frame-in_start_frame+1, 0, in_params.NumOfFrames)),
		frame_size(in_params.RawFrameSize),
		n_b_rays(in_params.NumOfBBeams),
		n_b_samples(in_params.SizeofBBeamAtSamples),
		cfm_density(in_params.CFMDensity),
		first_cfm_ray(in_params.FirstScanCFMBeam),
		first_cfm_sample(in_params.NumOfFirstCFMSample),
		cfm_filter_order(in_params.CFMFilterOrder)
{

	b_frames.realloc({n_frames, n_b_rays, n_b_samples});
	cfm_frames.realloc({n_frames, n_cfm_shots()*n_cfm_beams(), n_cfm_samples()});
	cfm_shot_frames.UseData(&cfm_frames.at({0,0,0}), {n_frames, n_cfm_beams(), n_cfm_shots(), n_cfm_samples()}, 1);


	double	x0 = double(first_cfm_ray)/n_b_rays;
	double	x1 = x0 + double(n_cfm_beams()*cfm_density)/n_b_rays;
	double	y0 = double(first_cfm_sample)/n_b_samples;
	double	y1 = y0 + double(n_cfm_samples())/n_b_samples;

	size_t	geometry = Decide("Select probe geometry", /*3,*/ {"Linear", "Convex", "Cardio"});

	if(geometry)
	{
		if(geometry==1)
			sco_b = ScanFrameSector(cm(6), cm(15), degrees(-34), degrees(34));
		else
			sco_b = ScanFrameSector(cm(0.1), cm(15), degrees(-60), degrees(60));

		physical_length	path_length_at_cfm_start = sco_b.scanning_trajectory_length() +
								first_cfm_sample*(sco_b.depth_range()/n_b_samples)*sco_b.angle_range().radians();

		sco_cfm = ScanFrameSector(path_length_at_cfm_start*(x1-x0), sco_b.depth_range()*(y1-y0),
								  sco_b.start_angle() + x0*(sco_b.end_angle() - sco_b.start_angle()),
								  sco_b.start_angle() + x1*(sco_b.end_angle() - sco_b.start_angle()));
		sco_b.pixels_per_cm = 30;
		sco_cfm.pixels_per_cm = 30;
	}
	else
	{
		sco_b = ScanFrameRectangle(cm(4), cm(5));
		sco_cfm = ScanFrameRectangle(sco_b.scanning_trajectory_length()*(x1-x0), sco_b.depth_range()*(y1-y0));
		sco_b.pixels_per_cm = 60;
		sco_cfm.pixels_per_cm = 60;
	}
}


void	S500_CFMFrameSet::ReadAllFrames(bool unsweep)
{
	shared_cfile dat_file;
	dat_file.open(file_params.dat_filename.c_str(), L"rb");
	GUIProgressBar	progress;
	progress.start("Reading frames", n_frames);

	for(size_t i = 0; i < n_frames; ++i)
	{
		ReadFrame(dat_file, i+start_frame, unsweep);
		++progress;
	}
}


void	S500_CFMFrameSet::ReadFrame(shared_cfile file, size_t frame_no, bool unsweep)
{
	index_vector	iv ={frame_no - start_frame, slice_mask(0), slice_mask(1)};
	b_slice_t	b_slice;
	cfm_slice_t	cfm_slice;
	b_frames.GetSlice(b_slice, iv);
	cfm_frames.GetSlice(cfm_slice, iv);
	DataArray<char>	header_buffer(file_params.HeaderSize);

//	значения ниже не используются, оставляю в комментарии для справки
//	size_t	b_frame_size = n_b_rays * (n_b_samples*2*sizeof(int32_t) + file_params.HeaderSize);	
//	size_t	cfm_frame_size = n_cfm_shots*n_cfm_beams * (n_cfm_samples*2*sizeof(int32_t) + file_params.HeaderSize);
	file.seek(frame_no * (frame_size), SEEK_SET);

	for(size_t i = 0; i < n_b_rays; ++i)
	{
		file.read_numbers(header_buffer, ioUI8);
		file.read_numbers(b_slice.row(i), ioComplexI32_LE);
	}
	for(size_t i = 0; i < n_cfm_shots()*n_cfm_beams(); ++i)
	{
		size_t	target_row_no = ReorderRays(i, unsweep);	
		file.read_numbers(header_buffer, ioUI8);
		file.read_numbers(cfm_slice.row(target_row_no), ioComplexI32_LE);
	}
}

XRAD_END


