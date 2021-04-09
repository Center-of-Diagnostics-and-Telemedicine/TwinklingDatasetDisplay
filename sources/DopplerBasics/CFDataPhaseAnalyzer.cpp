#include "pre.h"

#include <omp.h>

#include <XRADBasic/Sources/Utils/TimeProfiler.h>
#include <XRADBasic/Sources/Utils/ImageUtils.h>
#include <XRADBasic/Sources/Utils/ExponentialBlurAlgorithms.h>
#include <XRADBasic/Sources/Containers/DataArrayAnalyzeMD.h>

#include <DopplerBasics/CFDataPhaseAnalyzer.h>
#include <DopplerBasics/Utils/Auxiliaries.h>
//#include <DopplerBasics/NeutralizeTissueMotion.h>

//#include <DopplerBasics/WallFiltersInterface.h>


XRAD_BEGIN


#if 1
CFDataPhaseAnalyzer::CFDataPhaseAnalyzer() : 
	frames_buffer_sizes(3, 0)

{
	SetCFMFrameSizes(frames_buffer_sizes, 0, 0);

	frame_agility_factor = 0.25;
	rf_axial_blur = 7.5;
#pragma message Точно ли эта величина здесь нужна??
//	result_aspect_ratio = 5;// продокументировать, почему именно такое значение
	result_lateral_blur = 1;
//	acceleration_flag = false;
}
#endif
//TODO Несколько функций постфильтрации, которые стоит унифицировать


// void	PostfilterFrame(RealFunction2D_F32 &slice, double axial_blur)
// {
// 	// коррекция систематических изменений параметра с изменением глубины. Название неудачно
// 
// 	CutHistogramEdges(slice, range1_F64(0.01, 0.99));
// 
// #pragma message эта функция мешается с AcquireFrame, унифицировать
// 	BiexpBlur2D(slice, 0, axial_blur);
// }
// 



// Конец блока функций постфильтрации




void CFDataPhaseAnalyzer::FlushBuffers()
{
	//	Метод ничего существенного в данных не меняет. 
	//	Его задача -- добиться того, чтобы перед обработкой используемая память гарантированно была 
	//	не в свопе. Иначе при обработке первого кадра возможна потеря быстродействия.

	// при аллокировании память выделена только формально.
	// реальные ячейки реальной памяти выделяются только при первом
	// обращении к ней. это занимает время. в результате оказалось,
	// что первый кадр обрабатывается 30--50 мс, последующие - только 8--10.
	// при этом хочется, чтобы данные для закачки были "нетривиальными",
	// т.к. простое заполнение нулями она может как-то "соптимизировать"
	// поэтому действие повторяется несколько раз.


	// Прокачка ссылочного массива shot_slices (меняются данные cfm_frame)
	#pragma omp parallel for schedule (guided)
	for(ptrdiff_t shot = 0; shot < n_cfm_shots(); ++shot)
	{
		shot_slices[shot].fill(complexF64(shot));
	}

	// Прокачка ссылочного массива shots_array (меняются данные cfm_frame)
	#pragma omp parallel for schedule (guided)
	for(ptrdiff_t beam = 0; beam < n_cfm_beams(); ++beam)
	{
		for(size_t sample = 0; sample < n_cfm_samples(); ++sample)
		{
			shots_array.at(beam,sample).fill(complexF64(beam, sample));
		}
	}

	// инициализация нулями ниже необходима уже для обработки
	cfm_frame.fill(complexF64(0));
}


void CFDataPhaseAnalyzer::PrepareBuffers(const index_vector &new_frames_buffer_sizes, size_t in_beams_in_sweep, size_t in_ray_header_size_in_bytes)
{
	bool b_reallocate = new_frames_buffer_sizes != frames_buffer_sizes;

	SetCFMFrameSizes(new_frames_buffer_sizes, in_beams_in_sweep, in_ray_header_size_in_bytes);

	if(b_reallocate)
	{

		m_stddev.realloc(n_cfm_beams(), n_cfm_samples());
		m_correlation.realloc(n_cfm_beams(), n_cfm_samples());
		m_correlation_re_im.realloc(n_cfm_beams(), n_cfm_samples());
		m_amplitude.realloc(n_cfm_beams(), n_cfm_samples());
		m_phase.realloc(n_cfm_beams(), n_cfm_samples());
		m_mask.realloc(n_cfm_beams(), n_cfm_samples());
		cfm_frame.realloc(new_frames_buffer_sizes, complexF64(0));

		// формирование ссылочного массива shot_slices
		shot_slices.realloc(n_cfm_shots());
		for(size_t shot = 0; shot < n_cfm_shots(); ++shot)
		{
			index_vector	slice_iv(3);
			slice_iv[ray_index] = slice_mask(0);
			slice_iv[sample_index] = slice_mask(1);
			slice_iv[shot_index] = shot;
			shot_slices[shot] = cfm_frame.GetSlice(slice_iv);
		}

		// формирование ссылочного массива shot_array
		shots_array.realloc(n_cfm_beams(), n_cfm_samples());
		for(size_t beam = 0; beam < n_cfm_beams(); ++beam)
		{
			index_vector	shot_iv(3);
			shot_iv[ray_index] = beam;
			shot_iv[shot_index] = slice_mask(0);
			shot_iv[sample_index] = 0;
			for(size_t sample = 0; sample < n_cfm_samples(); ++sample, ++shot_iv[sample_index])
			{
				shots_array.at(beam,sample) = cfm_frame.GetRow(shot_iv);
			}
		}

		frames_buffer_sizes = cfm_frame.sizes();
	}

	FlushBuffers();
	ResetFrameAveraging();
}

void CFDataPhaseAnalyzer::ImportRawData(const cfm_container_t::value_type *frames_pointer, bool unsweep)
{
#ifndef _DEBUG
#pragma omp parallel for schedule (guided)
#endif
	for(ptrdiff_t source_shot_no = 0; source_shot_no < n_cfm_shots(); ++source_shot_no)
	{
		DataArray<const cfm_container_t::value_type>	source_row;
		cfm_array_t	target_row;

		for(size_t source_row_no = 0; source_row_no < n_cfm_beams(); ++source_row_no)
		{
			size_t	global_source_row_no = source_row_no*n_cfm_shots() + source_shot_no;
			size_t	global_target_row_no = ReorderRays(global_source_row_no, unsweep);

			size_t	target_shot_no = global_target_row_no % n_cfm_shots();
			size_t	target_row_no  = global_target_row_no / n_cfm_shots();

			index_vector	iv = { target_row_no, target_shot_no, slice_mask(0) };
			cfm_frame.GetRow(target_row, iv);

			const char* bytewise_pointer = reinterpret_cast<const char*>(frames_pointer);
//			вариант, при котором вызывающая процедура указывает на первый байт данных включая заголовок:
// 			size_t	byte_offset = global_source_row_no*(n_cfm_samples*sizeof(cfm_container_t::value_type) + ray_header_size_in_bytes) + ray_header_size_in_bytes;
//			вариант, при котором вызывающая процедура указывает на первый байт данных после первого заголовка:
			size_t	byte_offset = global_source_row_no*(n_cfm_samples() * sizeof(cfm_container_t::value_type)+ ray_header_size_in_bytes());
			const cfm_container_t::value_type *ray_start_pointer = reinterpret_cast<const cfm_container_t::value_type*>(bytewise_pointer + byte_offset);

			source_row.UseData(ray_start_pointer, n_cfm_samples());
			CopyData(target_row, source_row);
		}
	}
	m_conjugated_count = false;
}

void CFDataPhaseAnalyzer::ImportOrderedData(const cfm_container_t &current_cfm_frames)
{
#pragma omp parallel for schedule (guided)
	for(ptrdiff_t i = 0; i < n_cfm_shots(); ++i)
	{
		index_vector	iv = { slice_mask(0), size_t(i), slice_mask(1) };
		cfm_container_t::slice_type	original_slice;
		shot_slice_t	internal_slice;

		cfm_frame.GetSlice(internal_slice, iv);
		const_cast<cfm_container_t*>(&current_cfm_frames)->GetSlice(original_slice, iv);
		for(size_t j = 0; j < internal_slice.vsize(); ++j)
		{
			CopyData(internal_slice.row(j), original_slice.row(j));
		}
	}
}


void CFDataPhaseAnalyzer::ConjugateRFData()
{
	tp_conjugate.Start();
#if 0
#pragma message see comment
// использование простого сопряженного умножения чрезмерно расширяет динамический диапазон,
// что приводит к усилению артефактов, связанных с яркими объектами.
// следует использовать оператор, идущий ниже под #else
	percent_assign<complex_qm<float>, complex_qm<float> > percent_f;
#else
	low_contrast_conjugate percent_f;
#endif

	XRAD_ASSERT_THROW(n_cfm_shots()-1 > conjugated_count());

	#pragma omp parallel for schedule (guided)
	for(ptrdiff_t i = 0; i < n_cfm_beams(); ++i)
	{
		auto	last_shot = n_cfm_shots() - conjugated_count() - 1;
		for(size_t shot = 0; shot < last_shot; ++shot)
		{
			Apply_AA_1D_F2(shot_slices[shot].row(i), shot_slices[shot + 1].row(i), percent_f);
		}
		for(size_t shot = last_shot; shot < n_cfm_shots(); ++shot)
		{
			shot_slices[shot].row(i).CopyData(shot_slices[shot-1].row(i));
		}
	}
	++m_conjugated_count;
	tp_conjugate.Stop();
}

void CFDataPhaseAnalyzer::BlurRFData()
{
	tp_blur.Start();
	#pragma omp parallel for schedule (guided)
	for(ptrdiff_t shot = 0; shot < n_cfm_shots()-1; ++shot)
	{
	// поперечное размытие здесь практически ничего не дает,
	// только потеря темпа. поэтому первый размер 0
		BiexpBlur2D(shot_slices[shot], 0, rf_axial_blur*result_axial_blur);
	}
	tp_blur.Stop();
}


void CFDataPhaseAnalyzer::ChangeLastBeam(RealFunction2D_F32 &elastogram)
{
	for (size_t beam_no = 0; (beam_no + 1) < n_cfm_beams(); ++beam_no)
	{
		if ((beam_no + 1) % n_cfm_beams_in_sweep() == 0)
		{
			elastogram.row(beam_no + 1).CopyData(elastogram.row(beam_no));
		}
	}
}




void CFDataPhaseAnalyzer::ResetFrameAveraging()
{

	m_stddev.reset();
	m_correlation.reset();
	m_correlation_re_im.reset();
	m_amplitude.reset();
	m_phase.reset();
	m_mask.reset();
}


void	CFDataPhaseAnalyzer::ComputePreWallFilterAmplitude()
{
#pragma omp parallel for schedule (guided)
// Лучи одного свипа должны обрабатываться подряд друг за другом. Поэтому цикл делится на два вложенных
	for(ptrdiff_t sweep_no = 0; sweep_no < ptrdiff_t(n_cfm_sweeps()); ++sweep_no)
	{
		for(ptrdiff_t sbeam_no = 0; sbeam_no < ptrdiff_t(n_cfm_beams_in_sweep()); ++sbeam_no)
		{
			size_t beam_no = sweep_no * n_cfm_beams_in_sweep() + sbeam_no;
			if(first_beam_in_large_sweep(beam_no))
			{
				CopyData(m_amplitude.row(beam_no), m_amplitude.row(beam_no-1));
			}
			else
			{
				CopyData(m_amplitude.row(beam_no), shots_array.row(beam_no), [](auto& y, const auto& x){return y = sqrt(AverageValueTransformed(x, [](const auto& xx){return quadratic_norma(xx);}));});
			}
		}
	}

	m_amplitude /= AverageValue(m_amplitude);
	m_amplitude.AcquireFrame(frame_agility_factor, result_axial_blur, result_lateral_blur);
}


void	CFDataPhaseAnalyzer::AddNoise()
{
	for(size_t i = 0; i < n_cfm_beams(); ++i)
	{
		// цикл по лучам ЦДК
		auto shots_row_it = shots_array.row(i).begin();
		auto shots_row_ie = shots_array.row(i).end();
		for(size_t j = 0; shots_row_it < shots_row_ie; ++shots_row_it, ++j)
		{
			// цикл по глубинам ЦДК
			auto	it = shots_row_it->begin();
			auto	ie = it + n_cfm_shots();
			ComplexFunctionF32 burst(n_cfm_shots(), complexF32(0));
			size_t signma = 28000;  //10000~50power~7dB;   28000~0~4dB
			std::copy(it, ie, burst.begin());
			for(size_t k = 0; k < n_cfm_shots(); ++k)
			{
				burst[k] += complexF32(RandomGaussian(0, signma), RandomGaussian(0, signma));
			}
			std::copy(burst.begin(), burst.end(), it);
		}
	}
}



//---------------------------------------------------------------------

XRAD_END