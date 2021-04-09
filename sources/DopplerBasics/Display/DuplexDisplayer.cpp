#include "pre.h"
#include <XRADBasic/Sources/Utils/Crayons.h>
#include "DuplexDisplayer.h"
#include <DopplerBasics/CFDataPhaseAnalyzer.h>

#include "ColorFlowPalettes.h"

#include <omp.h>

XRAD_BEGIN

//-----------------------------------------------------------
//
// инициализация
//
DuplexDisplayer::DuplexDisplayer(const S500_CFMFrameSet &in_fs, const RealFunctionMD_F32 &in_offsets, const RealFunctionMD_F32 &in_mask_frames):
	fs(in_fs),
	cfm_frames(in_offsets),
	mask_frames(in_mask_frames)
{
	interpolator.InitFilters(16, 16, BSplineFilterGenerator2D<FIRFilter2DReal>(1));

//	offsets.realloc({ fs.n_frames, fs.n_cfm_beams, fs.n_cfm_samples });
	window_size_s = fs.n_cfm_samples();// размер эластограммы вдоль луча
	window_size_r = fs.n_cfm_beams();// размер эластограммы поперек луча

//	mask_frames.realloc({ fs.n_frames, fs.n_cfm_beams, fs.n_cfm_samples });

	cfm_density_r = fs.cfm_density;
	cfm_density_s = 1;
	window_corner_r = fs.first_cfm_ray;
	window_corner_s = fs.first_cfm_sample - fs.cfm_filter_order/2;
}

//-----------------------------------------------------------
//
// показ результата
//


class	db_functor
{
	double	maxval, log_maxval, dyn_range;
	double	minval;

	template<class T>
	T	internal_log(const T& x) const
	{
		return log10(x) - log_maxval;
	}

	template<class T, class ST>
	double	internal_log(const ComplexSample<T,ST> &x) const
	{
		return 0.5*log10(cabs2(x)) - log_maxval;
	}

public:
	db_functor(double mx, double d) : maxval(mx), dyn_range(d)
	{
		minval = maxval * pow(10,-dyn_range/20);
		log_maxval = log10(maxval);
	}

	double operator()(const double &x) const
	{
		return (x < minval ? 0 : 20*internal_log(x)+dyn_range) * 255./dyn_range;
	}
	template<class T1, class T2>
	T1 &operator()(T1 &y, const T2 &x) const
	{
		return y = (x < minval ? 0 : 20*internal_log(x)+dyn_range) * 255./dyn_range;
	}
};

struct cabs_functor
{
	using result_type = float;
	result_type operator()(result_type &y, const complexF32 &x) const { return y = cabs(x); }
};

template<class ARR>
ColorImageF32	DuplexDisplayer::DrawDuplex(
	const ARR &mask_cfm, const ARR &slice_cfm,
	GradientPalette &palette,
	ColorScanConverter &SC,
	bool just_frame)
{
	const double	max_cfm_value = 1;//MaxValue(offsets);
	const double	min_cfm_value = 0;

	size_t n_covered_rays = window_size_r*cfm_density_r;
	size_t n_covered_samples = window_size_s*cfm_density_s;

	// параметры для рисования желтой рамки
	physical_length frame_thickness = SC.depth_range()/150;
	int	axial_frame_width = frame_thickness * double(SC.n_samples()) / SC.depth_range();
	physical_length r0 = SC.angle_range().radians() ?  SC.scanning_trajectory_length()/SC.angle_range().radians() : mm(0);

	#pragma omp parallel for schedule (guided)
	for(int i = 0; i < int(n_covered_rays); ++i)
	{
		int	ray = range(i + window_corner_r, 0, int(SC.vsize()-1));
		double	cfm_ray = double(i)/cfm_density_r;

		ColorFunctionF32::iterator it = SC.row(ray).begin() + range(window_corner_s, 0, SC.hsize()-1);
		ColorFunctionF32::iterator ie = range(it+n_covered_samples, it, SC.row(ray).end());

		for(int j = 0; it<ie; ++j, ++it)
		{
			if(!just_frame)
			{
				double cfm_sample = double(j)/cfm_density_s;
				double opacity(mask_cfm.in(cfm_ray, cfm_sample, &interpolator));

				uint8_t	color_index = 255 * (slice_cfm.in(cfm_ray, cfm_sample, &interpolator) - min_cfm_value) / (max_cfm_value - min_cfm_value);
				*it *= (1. - opacity);
				*it += palette(color_index)*opacity;
			}

			auto	r1 = SC.depth_range() * (double(j + window_corner_s)/SC.n_samples());
			double	narrowing = r0.mm() ? r0/(r0 + r1) : 1;
			int	lateral_frame_width = frame_thickness * narrowing * double(SC.n_rays()) / SC.scanning_trajectory_length();

			if((i <= lateral_frame_width) || (i >= int(n_covered_rays - lateral_frame_width-1)) || (j <= axial_frame_width) || (j >= int(n_covered_samples - axial_frame_width)))
			{
				*it = crayons::yellow();
			}
		}
	}

	SC.BuildConvertedImage();
	return SC.GetConvertedImage();
}


void	DuplexDisplayer::DisplayDuplex()
{
	const double	max_cfm_value = 1;//MaxValue(offsets);
	const double	min_cfm_value = 0;

	RealFunctionMD_F32	b_frames(fs.b_frames.sizes());

	b_frames.CopyData(fs.b_frames, cabs_functor());
	BiexpBlur3D(b_frames, {1, 1, 1}, e_use_omp);


	
	
	double target_dynamic_range = GetFloating("Dynamic range for B frame", SavedGUIValue(60.), 0, 200);
	
	bool	grid = YesOrNo("Draw grid?", SavedGUIValue(false));
	physical_length grid_step = grid ? cm(GetFloating("Grid step in cm", SavedGUIValue(0.5), 0.1, 5)) : cm(0.5);
	
	db_functor db_compress_functor(MaxValue(b_frames), target_dynamic_range);

	ColorScanConverter	SC(fs.n_b_rays, fs.n_b_samples);

	//здесь грузится форма датчика
	SC.CopyScanConverterOptions(fs.sco_b);
	SC.InitScanConverter(512);//видимая высота картинки в пикселах
	SC.SetGrid(grid, ColorSampleF32(255), grid_step);
	// ниже идет фиктивный вызов, только для определения размеров растра
	SC.BuildConvertedImage();
	const size_t	frame_width = SC.GetConvertedImage().hsize();
	const size_t	frame_height = SC.GetConvertedImage().vsize();

	ColorImageMD_F32	double_rasters({fs.n_frames, frame_height, frame_width*2});
	
	TimeProfiler	tp;
	tp.Start();


	auto pt = Decide(L"Palette", 
		{
			MakeButton(L"Gray", cfm_palette::gray), 
			MakeButton(L"Medison",  cfm_palette::cfm_medison), 
			MakeButton(L"Medison 2",  cfm_palette::cfm_medison_2), 
			MakeButton(L"Сономед",  cfm_palette::cfm_sonomed),
			MakeButton(L"Twinkling",  cfm_palette::cfm_cascillation),
			MakeButton(L"Сustom", cfm_palette::custom)
		});

	GradientPalette	palette = GeneratePaletteCFM(pt);

	GUIProgressBar	progress;

	progress.start("Creating duplex animation", fs.n_frames);

	for(size_t frame = 0; frame < fs.n_frames; ++frame)
	{
		auto	index ={frame, slice_mask(0),slice_mask(1)};
		auto slice_cfm = cfm_frames.GetSlice(index);
		CopyData(SC, b_frames.GetSlice(index), db_compress_functor);
				
		SC.BuildConvertedImage();
		auto mask_cfm = mask_frames.GetSlice({ frame, slice_mask(0), slice_mask(1) });

		ColorImageF32	current_b_raster;
		ColorImageF32	current_duplex_raster;

		current_duplex_raster.UseData(&(double_rasters.at({frame, 0,0})), frame_height, frame_width, 2*frame_width, 1);
		current_b_raster.UseData(&(double_rasters.at({frame, 0, frame_width})), frame_height, frame_width, 2*frame_width, 1);

		current_b_raster.CopyData(DrawDuplex(mask_cfm, slice_cfm, palette, SC, true));
		current_duplex_raster.CopyData(DrawDuplex(mask_cfm, slice_cfm, palette, SC, false));

		for(size_t i = 0; i < 8; ++i)
		{
			for(size_t j = 0; j < 256; ++j)
			{
				current_duplex_raster.at(255 - j, frame_width - 1 - i) = palette(j);
			}
		}

		++progress;
	}
	tp.Stop();
	progress.end();

	string title = ssprintf("Duplex display, SC time %g ms (%g per frame)", tp.LastElapsed().msec(), tp.LastElapsed().msec()/fs.n_frames);
	DisplayMathFunction3D(double_rasters, title);
}




void	DuplexDisplayer::DisplayCFMResults()
{

	enum
	{
		e_duplex,
		e_transperency_mask,
		e_frame_animation,
		e_exit
	};

	try
	{
		while(true)
		{
			auto choice = GetButtonDecision("Choose display option",
			{
				"Duplex animation",
				"Mask animation",
				"Offsets animation",

				"Exit"
			});
			switch(choice)
			{
				case e_duplex:
					DisplayDuplex();
					break;

				case e_transperency_mask:
					DisplayMathFunction3D(mask_frames, "mask", fs.sco_cfm);
					break;

				case e_frame_animation:
					DisplayMathFunction3D(cfm_frames, "Offsets", fs.sco_cfm);
					break;

				case e_exit:
					throw canceled_operation("");
					break;
			}
		}
	}
	catch(canceled_operation){}
}

XRAD_END

