#include "pre.h"
#include <XRADBasic/ContainersAlgebra.h>
#include <XRADBasic/Sources/Utils/ExponentialBlurAlgorithms.h>
#include <XRADBasic/Sources/Utils/StatisticUtils.h>
#include <XRADBasic/Sources/Utils/ImageUtils.h>
#include <XRADBasic/Sources/Utils/LeastSquares.h>

#include <DopplerBasics/Utils/NeutralizeTissueMotion.h>

#include "CFDataPhaseAnalyzerCFM.h"

/********************************************************************
	created:	2016/10/20
	created:	20:10:2016   14:09
	filename: 	q:\programs\ElastoGrafica\ElastoGrafica\FrameCFMAnalyzer.cpp
	file path:	q:\programs\ElastoGrafica\ElastoGrafica
	file base:	FrameCFMAnalyzer
	file ext:	cpp
	author:		kns
	
	purpose:	
*********************************************************************/

XRAD_BEGIN



// функция находит коэффициенты попиксельного полиномиального преобразование изображения other, при котором среднеквадратичная разница original и other минимальна
// other_transformed.at(i,j) = sum_n(pow(other,n).at(i,j), coefficients[n]), n=0...n_coefficients-1
// временно эта функция здесь, она может пригодиться как заготовка для других проектов

RealVectorF64	brightness_match_transform(const RealFunction2D_F32  &original, const RealFunction2D_F32 &other, size_t n_components)
{
	auto flatten = [](const auto &data)
	{
		XRAD_ASSERT_THROW(data.hstep() == 1 && data.vstep() == ptrdiff_t(data.hsize()));
		auto ptr = data.data();
		RealFunctionF32::invariable result;
		result.UseData(ptr, data.element_count());

		return result;
	};

	RealVectorF64 coefficients(n_components);
	DetectLSPolynomNonUniformGrid(flatten(original), flatten(other), coefficients);

//	printf("\na0= %g, a1 = %g", coefficients[0], coefficients[1]);
	return coefficients;
}
//

void CFDataPhaseAnalyzerCFM::PrepareBuffers(const index_vector& cfm_frames_sizes, size_t in_beams_in_sweep, size_t in_ray_header_size_in_bytes)
{
	parent::PrepareBuffers(cfm_frames_sizes, in_beams_in_sweep, in_ray_header_size_in_bytes);

	m_bloodflow_mask.realloc(n_cfm_beams(), n_cfm_samples());
	m_cavitation_mask.realloc(n_cfm_beams(), n_cfm_samples());
	m_oscillation_mask.realloc(n_cfm_beams(), n_cfm_samples());

}

void CFDataPhaseAnalyzerCFM::ResetFrameAveraging()
{
	parent::ResetFrameAveraging();

	m_bloodflow_mask.reset();
	m_cavitation_mask.reset();
	m_oscillation_mask.reset();
}


//TODO разобраться с именами переменных (warning 4458)
void	CFDataPhaseAnalyzerCFM::CalculateMask()
{
	tp_build_mask.Start();

#if 0 //режим ЦДК обычный
	mask_buffer.fill(0);

	AppendMaskLayer(m_stddev_buffer, stddev_threshold, stddev_threshold/2);
	AppendMaskLayer(m_amplitude_buffer, amplitude_threshold, 0.1, -1);


#else // наш усложненный режим ЦДК


	mask_criteria_set	bloodflow_criteria;
	mask_criteria_set	oscillation_criteria;
	mask_criteria_set	cavitation_criteria;

	bloodflow_criteria.init_bloodflow(n_cfm_shots());
	oscillation_criteria.init_oscillation(n_cfm_shots());
	cavitation_criteria.init_cavitation(n_cfm_shots());

	ApplyMaskCriteriaSet(m_bloodflow_mask, bloodflow_criteria);
	ApplyMaskCriteriaSet(m_cavitation_mask, cavitation_criteria);
	ApplyMaskCriteriaSet(m_oscillation_mask, oscillation_criteria);

	#pragma omp parallel for schedule (guided)
	for(ptrdiff_t i = 0; i < ptrdiff_t(m_mask.vsize()); ++i)
	{
		for(size_t j = 0; j < m_mask.hsize(); ++j)
		{
			m_mask.at(i,j) = vmax(m_bloodflow_mask.at(i, j), m_cavitation_mask.at(i, j), m_oscillation_mask.at(i, j));
		}
	}

#endif
	tp_build_mask.Stop();
}


void CFDataPhaseAnalyzerCFM::ApplyMaskCriteriaSet(FrameAcquisitionBuffer &current_mask, const mask_criteria_set &mcs)
{
	size_t	counter(0);
	auto	apply_criterium = [&counter](auto &mask, const auto &criterium_data, const auto &criterium)
	{
		if(criterium.active())
		{
			Apply_AA_2D_F2(mask, criterium_data, criterium);
			++counter;
		}
	};
	current_mask.fill(0);
	if(m_phase.n_frames_acquired() > 1)
	{
		apply_criterium(current_mask, m_phase.dispersion(), mcs.phase_dispersion_criterium);
	}
	else
	{
		//current_mask += 1;// дает чрезмерную вспышку на первом кадре
	}


	apply_criterium(current_mask, m_correlation, mcs.correlation_criterium);
	apply_criterium(current_mask, m_stddev, mcs.stddev_criterium);
	apply_criterium(current_mask, m_stddev, mcs.stddev_cavitation_criterium);
	apply_criterium(current_mask, m_correlation_re_im, mcs.re_im_correlation_criterium);

	if(!counter) current_mask.fill(1);

	auto	mask_gate = [&mcs](float &m)
	{
		return m = soft_threshold(m, mcs.combined_mask_threshold, mcs.combined_mask_threshold_width);
	};

	//if(m_phase.n_frames_acquired() > 0) current_mask.AcquireFrame(frame_agility_factor, result_axial_blur, result_lateral_blur);
	// похоже, что без накопления выглядит "поживее"
	/*if(!CapsLock()) */
	ApplyFunction(current_mask, mask_gate);
}

void	CFDataPhaseAnalyzerCFM::AnalyzeFrame(RealFunction2D_F32 &result_flow_map, RealFunction2D_F32 &result_mask)
{
	
	// следующая маска не используется для анализа ЦДК, но может быть полезна для коррекции датасета.
	ComputePreWallFilterAmplitude();

	//TODO отключаю компенсацию движения ради производительности. 
	//	Отчасти артефакт вспышки компенсируется на этапе маскирования по re-im корреляции.
	//	Но требуется еще дополнительная проверка
	TissueMotionCompensation();
	
	if(!wall_filter) wall_filter = GetWallFilter(n_cfm_shots(), wall_filter_index);
	if(wall_filter->is_size_actual(n_cfm_shots())) wall_filter = GetWallFilter(n_cfm_shots(), wall_filter_index);

	ApplyWallFilter();

	tp_variadic.Start();
 	ComputeStandardDeviation();//2.47 ms
 	ComputeReImCorrelation(); //2.8 ms
	ComputeCorrelationCoefficient();//4.5 ms
	tp_variadic.Stop();

	CalculateMask();// вызывать до ConjugaterRFData, а не после

	ConjugateRFData();
//	BlurRFData();
	AcquirePhaseCFM();


	#pragma omp parallel for schedule (guided)
	for(ptrdiff_t i = 0; i < ptrdiff_t(m_mask.vsize()); ++i)
	{
		for(size_t j = 0; j < m_mask.hsize(); ++j)
		{
			auto& p(m_phase.at(i, j));
			auto& m(m_mask.at(i, j));
			auto& bfm(m_bloodflow_mask.at(i, j));
			auto& cavm(m_cavitation_mask.at(i, j));
			auto& oscm(m_oscillation_mask.at(i, j));
			auto& r(result_flow_map.at(i, j));

			if(bfm >= m)
			{
				const double	amplification = 1.5;//Усиление нужно, потому что из-за межкадрового сглаживания значения фазы уменьшаются. Коэффициент взят "на глаз"
				r = range((amplification*p+pi())/two_pi(), 0.1, 0.9);
			}
			else if(cavm >= m)
			{
				r = 1;
			}
			else if(oscm >= m)
			{
				r = 0;
			}
		}
	}


//	result_flow_map.CopyData(m_phase, phase_normalizer());
	result_mask.CopyData(m_mask);
}

void CFDataPhaseAnalyzerCFM::AcquirePhaseCFM()
{
	tp_acquire.Start();
	
	#pragma omp parallel for schedule (guided)
	for(ptrdiff_t i = 0; i < ptrdiff_t(n_cfm_beams()); ++i)
	{
	// цикл по лучам ЦДК
		auto shots_row_it = shots_array.row(i).begin();
		auto shots_row_ie = shots_array.row(i).end();
		auto phase_it = m_phase.row(i).begin();

		for(size_t shot = 0; shots_row_it<shots_row_ie; ++shots_row_it, ++phase_it, ++shot)
		{
		// цикл по глубинам ЦДК
			auto	it = shots_row_it->begin();
			auto	ie = it + (n_cfm_shots()-conjugated_count());
			complexF64	accumulator(0);
			//NB: проверено, что усреднение фазового аккумулятора по глубине ничего хорошего не дает
			for(; it < ie; ++it)
			{
			// цикл внутри "пачки"
				accumulator += *it;
			}
			double	current_offset = arg(accumulator);
			*phase_it = current_offset;
		}
	}
	
 	m_phase.AcquireFrame(frame_agility_factor, result_axial_blur, result_lateral_blur);
	tp_acquire.Stop();
};



void	CFDataPhaseAnalyzerCFM::TissueMotionCompensation()
{
	//TODO неясно, нужна ли специальная функция, или ее тело перенести сюда
	NeutralizeTissueMotionOneFrame(shots_array, n_cfm_beams_in_sweep(), n_cfm_shots(), m_tissue_motion_blur, b_neutralize_acceleration);
}


void	CFDataPhaseAnalyzerCFM::ApplyWallFilter()
{
	tp_wall_filter.Start();

	#pragma omp parallel for schedule (guided)
	for(ptrdiff_t i = 0; i < ptrdiff_t(n_cfm_beams()); ++i)
	{
		// цикл по лучам ЦДК
		auto shots_row_it = shots_array.row(i).begin();
		auto shots_row_ie = shots_array.row(i).end();
		for(size_t j = 0; shots_row_it < shots_row_ie; ++shots_row_it, ++j)
		{
			// цикл по глубинам ЦДК
			wall_filter->Apply(*shots_row_it);
		}
	}
	tp_wall_filter.Stop();
}

void	CFDataPhaseAnalyzerCFM::ComputeReImCorrelation()
{
	XRAD_ASSERT_THROW_M(!conjugated_count(), logic_error, "Function should be called begore conjugation")

	//	большой коэффициент корреляции: кровоток.
	//	малый: вероятно мерцание

	#pragma omp parallel for schedule (guided)
	// Лучи одного свипа должны обрабатываться подряд друг за другом. Поэтому цикл делится на два вложенных
	for(ptrdiff_t sweep_no = 0; sweep_no < ptrdiff_t(n_cfm_sweeps()); ++sweep_no)
	{
		for(ptrdiff_t sbeam_no = 0; sbeam_no < ptrdiff_t(n_cfm_beams_in_sweep()); ++sbeam_no)
		{
			size_t beam_no = sweep_no * n_cfm_beams_in_sweep() + sbeam_no;
			if(first_beam_in_large_sweep(beam_no))
			{
				// последний луч "свипа" отличается от других: иногда в нем возникают мешающие всплески
				m_correlation_re_im.row(beam_no).CopyData(m_correlation_re_im.row(beam_no-1));
			}
			else
			{
				// цикл по лучам ЦДК
				auto shots_row_it = shots_array.row(beam_no).begin();
				auto shots_row_ie = shots_array.row(beam_no).end();
				RealFunction2D_F32::row_type::iterator eit = m_correlation_re_im.row(beam_no).begin();
				for(; shots_row_it < shots_row_ie; ++shots_row_it, ++eit)
				{
					// цикл по глубинам ЦДК
					auto	it = shots_row_it->begin();
					auto	ie = it + (n_cfm_shots() - 1);
					double	d1(0), d2(0), numerator(0);
					for(; it < ie; ++it)
					{
						// цикл внутри "пачки"
						numerator += real(*it)*imag(*it);
						d1 += square(real(*it));
						d2 += square(imag(*it));
					}
					double denominator = sqrt(d1*d2);
					*eit = fabs(numerator / denominator);
				}
			}
		}
	}

	m_correlation_re_im.AcquireFrame(frame_agility_factor, result_axial_blur, result_lateral_blur);
}




void	CFDataPhaseAnalyzerCFM::ComputeStandardDeviation()
{
	XRAD_ASSERT_THROW_M(!conjugated_count(), logic_error, "Function should be called begore conjugation")
	// рост СКО: Признак "мерцания" или большой доплеровской скорости
	#pragma omp parallel for schedule (guided)
	// Лучи одного свипа должны обрабатываться подряд друг за другом. Поэтому цикл делится на два вложенных
	for(ptrdiff_t sweep_no = 0; sweep_no < ptrdiff_t(n_cfm_sweeps()); ++sweep_no)
	{
		for(ptrdiff_t sbeam_no = 0; sbeam_no < ptrdiff_t(n_cfm_beams_in_sweep()); ++sbeam_no)
		{
			size_t beam_no = sweep_no * n_cfm_beams_in_sweep() + sbeam_no;
			if(first_beam_in_large_sweep(beam_no))
			{
				// последний луч "свипа" отличается от других: иногда в нем возникают мешающие всплески
			m_stddev.row(beam_no).CopyData(m_stddev.row(beam_no-1));
			}
			else
			{
				auto shots_row_it = shots_array.row(beam_no).begin();
				auto shots_row_ie = shots_array.row(beam_no).end();
				RealFunction2D_F32::row_type::iterator mask_it = m_stddev.row(beam_no).begin();
				for(size_t j = 0; shots_row_it < shots_row_ie; ++shots_row_it, ++mask_it, ++j)
				{
					// цикл по глубинам ЦДК
					auto	it = shots_row_it->begin();
					auto	ie = it + (n_cfm_shots() - 1);
					double	cabs2_accumulator(0);
					for(; it < ie; ++it)
					{
						// цикл внутри "пачки"
						cabs2_accumulator += cabs2(*it);
					}

					*mask_it = sqrt(cabs2_accumulator / (n_cfm_shots() - 1));
				}
			}
		}
	}

	m_stddev /= AverageValue(m_stddev);
	m_stddev.AcquireFrame(frame_agility_factor, result_axial_blur, result_lateral_blur);
}

void	CFDataPhaseAnalyzerCFM::ComputeCorrelationCoefficient()
{
	XRAD_ASSERT_THROW_M(!conjugated_count(), logic_error, "Function should be called begore conjugation")
	//	большой коэффициент корреляции: кровоток.
	//	малый: вероятно мерцание

	#pragma omp parallel for schedule (guided)
	// Лучи одного свипа должны обрабатываться подряд друг за другом. Поэтому цикл делится на два вложенных
	for(ptrdiff_t sweep_no = 0; sweep_no < ptrdiff_t(n_cfm_sweeps()); ++sweep_no)
	{
		for(ptrdiff_t sbeam_no = 0; sbeam_no < ptrdiff_t(n_cfm_beams_in_sweep()); ++sbeam_no)
		{
			size_t beam_no = sweep_no * n_cfm_beams_in_sweep() + sbeam_no;
			if(first_beam_in_large_sweep(beam_no))
			{
			// последний луч "свипа" отличается от других: иногда в нем возникают мешающие всплески
			m_correlation.row(beam_no).CopyData(m_correlation.row(beam_no-1));
			}
			else
			{
			// цикл по лучам ЦДК
				auto shots_row_it = shots_array.row(beam_no).begin();
				auto shots_row_ie = shots_array.row(beam_no).end();
				RealFunction2D_F32::row_type::iterator eit = m_correlation.row(beam_no).begin();
				for(; shots_row_it < shots_row_ie; ++shots_row_it, ++eit)
				{
					// цикл по глубинам ЦДК
					auto	it = shots_row_it->begin();
					auto	it1 = it + 1;
					auto	ie = it + (n_cfm_shots() - 1);
					double	d1 = 0, d2 = 0;
					complexF64	numerator(0);
					for(; it < ie; ++it, ++it1)
					{
						// цикл внутри "пачки"
						numerator.add_multiply_conj(*it, *it1);
						d1 += cabs2(*it);
						d2 += cabs2(*it1);
					}
					double	denominator = sqrt(d1*d2);
					*eit = cabs(numerator / denominator);
				}
			}
		}
	}

	m_correlation.AcquireFrame(frame_agility_factor, result_axial_blur, result_lateral_blur);
}


XRAD_END
