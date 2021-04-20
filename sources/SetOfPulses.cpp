#include "pre.h"

#include <omp.h>

#include <RFDataImport/S500_CFMFrameSet.h>
#include <RFDataImport/S500_CFMRawDataDisplay.h>

#include <DopplerBasics/Utils/NeutralizeTissueMotion.h>
#include <DopplerBasics/CFM/WallFilters/WallFilters.h>
#include <DopplerBasics/CFM/WallFilters/WallFiltersInteractive.h>

#include "SetOfPulses.h"

#pragma message Часть функций отсюда перенести в Utils. Проверить, нет ли подобных действий в других режимах

XRAD_BEGIN

void	CalculateCorrelation(const ComplexFunctionF32 &burst, complexF64 &accumulator, double &divisor)
	{
	accumulator = complexF64(0);
	double	c1(0), c2(0);
	ComplexFunctionF32::const_iterator it1 = burst.begin(), it2 = burst.begin()+1, ie = burst.end();
	for(; it2 < ie; ++it1, ++it2)  
		{
		accumulator += *it1 % *it2;
		c1 += cabs2(*it1);
		c2 += cabs2(*it2);
		}
	divisor = sqrt(c1*c2);
	}

void	CalculateReImCorrelation(const ComplexFunctionF32 &burst, double &accumulator, double &divisor)
{
	accumulator = 0;
	double	c1(0), c2(0);
	ComplexFunctionF32::const_iterator it = burst.begin(), ie = burst.end();
	for (; it < ie; ++it)
	{
		accumulator += real(*it) * imag(*it);
		c1 += real(*it)*real(*it);
		c2 += imag(*it)*imag(*it);
	}
	divisor = sqrt(c1*c2);
}

void	CalculateSTD(const ComplexFunctionF32 &burst, double &std)
{
	double dispersion(0);//
	std = 0;
	ComplexFunctionF32::const_iterator it = burst.begin(), ie = burst.end();
	for (; it < ie; ++it)
	{
		dispersion += cabs(*it % *it);
	}
	std = sqrt(dispersion);
}

double	PhaseCalcNoWF(const ComplexFunctionF32 &burst, double mct)
{
	complexF64	accumulator;
	double divisor;
	CalculateCorrelation(burst, accumulator, divisor);
	double	cc = cabs(accumulator)/divisor;
	if(cc>mct)
		return arg(accumulator);
	else return 0;
}

double	CorCalcNoWF(const ComplexFunctionF32 &burst)
{
	complexF64	accumulator;
	double divisor;
	CalculateCorrelation(burst, accumulator, divisor);
	double	cc = cabs(accumulator) / divisor;
	return cc;
}

double	ReImCorCalcNoWF(const ComplexFunctionF32 &burst)
{
	double	accumulator;
	double divisor;
	CalculateReImCorrelation(burst, accumulator, divisor);
	double	cc = abs(accumulator) / divisor;
	return cc;
}

double	STDCalcNoWF(const ComplexFunctionF32 &burst)
{
	double	accumulator;
//	double divisor;
	CalculateSTD(burst, accumulator);
	return accumulator;
}

template<class WF>
double PhaseCalc(ComplexFunctionF32 &burst, const WF &wall_filter, double mct)
{
	wall_filter.Apply(burst);
	return PhaseCalcNoWF(burst, mct);
}

template<class WF>
double CorCalc(ComplexFunctionF32 &burst, const WF &wall_filter)
{
	wall_filter.Apply(burst);
	return CorCalcNoWF(burst);
}

template<class WF>
double ReImCorCalc(ComplexFunctionF32 &burst, const WF &wall_filter)
{
	wall_filter.Apply(burst);
	return ReImCorCalcNoWF(burst);
}

template<class WF>
double STDCalc(ComplexFunctionF32 &burst, const WF &wall_filter)
{
	wall_filter.Apply(burst);
	return STDCalcNoWF(burst);
}

void	CalculatePhase(const cfm_slice_t &cfm_slice, RealFunctionMD_F32::slice_type &phase, const WallFilter &wall_filter, double mct)
{
	size_t	n_shots = cfm_slice.vsize()/phase.vsize();
	size_t	n_rays = phase.vsize();
	size_t n_samples = phase.hsize();
	ComplexFunctionMD_F32::slice_type	cfm_slice_prepared(cfm_slice);
	for(size_t i = 0; i < n_samples; ++i)  
	{
		ComplexFunctionF32::const_iterator	it = cfm_slice_prepared.col(i).cbegin();
		for(size_t beam_no = 0; beam_no < n_rays; ++beam_no)  
		{
			ComplexFunctionF32 burst(n_shots);
			for(size_t shot_no = 0; shot_no < n_shots; ++shot_no, ++it)
				burst[shot_no]= *it;
			phase.at(beam_no,i)=PhaseCalc(burst, wall_filter, mct);
		}
	}
}

void	CalculateCorrelationMatrix(const cfm_slice_t &cfm_slice, RealFunctionMD_F32::slice_type &correlation, const WallFilter &wall_filter)
{
	size_t	n_shots = cfm_slice.vsize() / correlation.vsize();
	size_t	n_rays = correlation.vsize();
	size_t n_samples = correlation.hsize();
	ComplexFunctionMD_F32::slice_type	cfm_slice_prepared(cfm_slice);
	for (size_t i = 0; i < n_samples; ++i)
	{
		ComplexFunctionF32::const_iterator	it = cfm_slice_prepared.col(i).cbegin();
		for (size_t beam_no = 0; beam_no < n_rays; ++beam_no)
		{
			ComplexFunctionF32 burst(n_shots);
			for (size_t shot_no = 0; shot_no < n_shots; ++shot_no, ++it)
				burst[shot_no] = *it;
			correlation.at(beam_no,i) = CorCalc(burst, wall_filter);
		}
	}
}

void	CalculateReImCorrelationMatrix(const cfm_slice_t &cfm_slice, RealFunctionMD_F32::slice_type &re_im_correlation, const WallFilter &wall_filter)
{
	size_t	n_shots = cfm_slice.vsize() / re_im_correlation.vsize();
	size_t	n_rays = re_im_correlation.vsize();
	size_t n_samples = re_im_correlation.hsize();
	ComplexFunctionMD_F32::slice_type	cfm_slice_prepared(cfm_slice);
	for (size_t i = 0; i < n_samples; ++i)
	{
		ComplexFunctionF32::const_iterator	it = cfm_slice_prepared.col(i).cbegin();
		for (size_t beam_no = 0; beam_no < n_rays; ++beam_no)
		{
			ComplexFunctionF32 burst(n_shots);
			for (size_t shot_no = 0; shot_no < n_shots; ++shot_no, ++it)
				burst[shot_no] = *it;
			re_im_correlation.at(beam_no,i) = ReImCorCalc(burst, wall_filter);
		}
	}
}

void	CalculateSTDMatrix(const cfm_slice_t &cfm_slice, RealFunctionMD_F32::slice_type &re_im_correlation, const WallFilter &wall_filter)
{
	size_t	n_shots = cfm_slice.vsize() / re_im_correlation.vsize();
	size_t	n_rays = re_im_correlation.vsize();
	size_t n_samples = re_im_correlation.hsize();
	ComplexFunctionMD_F32::slice_type	cfm_slice_prepared(cfm_slice);
	for (size_t i = 0; i < n_samples; ++i)
	{
		ComplexFunctionF32::const_iterator	it = cfm_slice_prepared.col(i).cbegin();
		for (size_t beam_no = 0; beam_no < n_rays; ++beam_no)
		{
			ComplexFunctionF32 burst(n_shots);
			for (size_t shot_no = 0; shot_no < n_shots; ++shot_no, ++it)
				burst[shot_no] = *it;
			re_im_correlation.at(beam_no,i) = STDCalc(burst, wall_filter);
		}
	}
}

size_t CountPixels(RealFunctionMD_F32::slice_type &phase)
{
	size_t res(0);
	for(size_t i = 0; i < phase.vsize(); ++i)   //горизонталь
	{
		for(size_t j = 0; j < phase.hsize(); ++j)
		{
			double c = phase.at(i,j);
			if(c != 0)
				res += 1;
		}
	}
	return res;
}

void	DisplayASetOfPulses(const S500_CFMFrameSet &frames, size_t frame_no)
{
	bool flag_tissue_motion_compenstion(true);// false);
	if (!GetCheckboxDecision("Choose PreProcessing Type", { "Tissue Motion Compensation" }, { &flag_tissue_motion_compenstion })) {}
	cfm_slice_t::invariable	cfm_slicet;
	cfm_container_t::invariable cfm_frame_by_shots_original;

	frames.cfm_frames.GetSlice(cfm_slicet, {frame_no, slice_mask(0), slice_mask(1)});
	frames.cfm_shot_frames.GetSubset(cfm_frame_by_shots_original, {frame_no, slice_mask(0), slice_mask(1), slice_mask(2)});

	ComplexFunction2D_F32	cfm_slice(cfm_slicet);
	ComplexFunctionMD_F32	cfm_frame_by_shots(cfm_frame_by_shots_original);

	
	size_t first_beam_no = GetUnsigned("Get First Beam No", MakeGUIValue(30, saved_default_value), 0, frames.n_cfm_beams()-1);  //28
	size_t last_beam_no = GetUnsigned("Get Last Beam No", first_beam_no, 0, frames.n_cfm_beams()-1);  //32  // frames.n_cfm_beams()-1;
	size_t length_in_beams = last_beam_no - first_beam_no + 1;
	size_t first_sample_no = GetUnsigned("Get First Sample No", MakeGUIValue(200, saved_default_value), 0, cfm_slice.hsize()-1);  //280
	size_t last_sample_no = GetUnsigned("Get Last Sample No", first_sample_no, 0, cfm_slice.hsize()-1); //320 //cfm_slice.hsize()-1; 
	size_t length_in_samples = last_sample_no - first_sample_no + 1;
	ComplexFunction2D_F32 region(frames.n_cfm_shots()*length_in_beams, length_in_samples);
	ComplexFunctionF32 burst(frames.n_cfm_shots());

	WallFilterPtr	wall_filter = GetWallFilterInteractive(frames.n_cfm_shots());
	CFDataPhaseAnalyzer::shots_array_t	shots_array(frames.n_cfm_beams(), cfm_slice.hsize());
	for (size_t beam_no = 0; beam_no < frames.n_cfm_beams(); ++beam_no)
	{
		for (size_t sample_no = 0; sample_no < cfm_slice.hsize(); ++sample_no)
		{
			cfm_frame_by_shots.GetRow(shots_array.at(beam_no, sample_no), { beam_no, slice_mask(0), sample_no });
		}
	}
	if (flag_tissue_motion_compenstion == true)
	{
		size_t accumulator_blur = GetUnsigned("Accumulator Blur", 5, 0, 1000);
		bool acceleration_flag = YesOrNo("acceleration compensation", false);
				
		TimeProfiler	tp;
		tp.Start();

		NeutralizeTissueMotionOneFrame(shots_array, frames.n_cfm_beams_in_sweep(), frames.n_cfm_shots(), accumulator_blur, acceleration_flag);
		tp.Stop();
	}
	for(size_t sample_no = 0; sample_no < length_in_samples; ++sample_no)   //горизонталь
	{
		for (size_t beam_no = 0; beam_no < length_in_beams; ++beam_no)
		{	
			for (size_t shot_no = 0; shot_no < burst.size(); ++shot_no)
			{
				//cfm_frame_by_shots.GetRow(burst, { first_beam_no + beam_no, slice_mask(0), first_sample_no + sample_no });
				burst[shot_no] = shots_array.at(first_beam_no + beam_no, first_sample_no + sample_no)[shot_no];
			}
			wall_filter->Apply(burst);
			for (size_t shot_no = 0; shot_no < burst.size(); ++shot_no)
			{
				region.at(shot_no +beam_no*burst.size(),sample_no)=burst[shot_no];
			}
		}
	}
	//if (burst.size() < size_t(frames.n_cfm_shots())) region.resize(burst.size()*length_in_beams, length_in_samples);
	DisplayMathFunction2D(region, ssprintf("CFM-frame %d", frame_no));
	RealFunction2D_F32 phase(length_in_beams, length_in_samples);
	WallFilterNone no_wall_filter;
	
	
	
	size_t	option = 0;
	enum { display_phase, diaplay_correlation, display_re_im_correlation, display_std, count_pixels, display_exit, n_options };
	do
	{
		option = GetButtonDecision("Choose filter type", { "Phase", "Correlation", "Re-Im Correlation", "STD", "Count Pixels", "Exit" });
		try
		{
			switch (option)
			{
			case display_phase:
			{
				double mct = 0;
				CalculatePhase(region, phase, no_wall_filter, mct);
				double mean_phase(0);
				for (size_t i=0;i<length_in_beams;i++)
				{
					for (size_t j = 0; j < length_in_samples; j++)
					{
						mean_phase += phase.at(i,j) / (length_in_beams*length_in_samples);
					}
				}
				ShowText("mean_phase", ssprintf("mean_phase %g", mean_phase), true);
				
				DisplayMathFunction2D(phase, ssprintf("Phase %d", frame_no));
			}
			break;
			case diaplay_correlation:
			{
				CalculateCorrelationMatrix(region, phase, no_wall_filter);
				DisplayMathFunction2D(phase, ssprintf("Correlation %d", frame_no));
			}
			break;
			case display_re_im_correlation:
			{
				CalculateReImCorrelationMatrix(region, phase, no_wall_filter);
				DisplayMathFunction2D(phase, ssprintf("Correlation %d", frame_no));
			}
			break;
			case display_std:
			{
				CalculateSTDMatrix(region, phase, no_wall_filter);
				DisplayMathFunction2D(phase, ssprintf("Correlation %d", frame_no));
			}
			break;
			case count_pixels:
			{
				double mct = GetUnsigned("Min Correlation Threshold", 0.9, 0, 1);
				CalculatePhase(region, phase, no_wall_filter, mct);
				DisplayMathFunction2D(phase, ssprintf("ProcessedCFM frame %d", frame_no));
				size_t pixels_in_region = CountPixels(phase);
				phase.realloc(frames.n_cfm_beams(), cfm_slice.hsize());
				CalculatePhase(cfm_slice, phase, *wall_filter, mct);
				if (CapsLock())
				{
					size_t pixels_outside_region = CountPixels(phase) - pixels_in_region;
					ShowText("Pixels", ssprintf("Frame %d. Pixels in region:  %d. Pixels outside:  %d.", frame_no, pixels_in_region, pixels_outside_region), true);
				}
			}
			break;
			case display_exit:
				break;
			}
		}
	catch (canceled_operation &) {}
	catch (quit_application &ex) { throw ex; }
	catch (exception &ex) { Error(ex.what()); }
	catch (...) { Error("An unknown exception"); }
	}while (option != n_options - 1);
}

XRAD_END


