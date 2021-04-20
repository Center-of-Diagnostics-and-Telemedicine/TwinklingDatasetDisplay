#ifndef __PhaseElastoFrameAnalyzer_h
#define __PhaseElastoFrameAnalyzer_h

//------------------------------------------------------------------
//
//	created:	2014/06/07
//	created:	7.6.2014   14:26
//	filename: 	Q:\programs\ElastographyTest\sources\PhaseElastoFrameAnalyzer.h
//	file path:	Q:\programs\ElastographyTest\sources
//	author:		kns
//	
//	purpose:	
//
//------------------------------------------------------------------



#include <RFDataImport/S500_CFMFrameSetParams.h>
//#include "DopplerDataTypes.h"
#include <RFDataImport/S500_CFMDataTypes.h>

#include <DopplerBasics/CFMModes.h>

#include "FrameAcquisitionBuffer.h"
#include <DopplerBasics/Utils/PhaseAnalyzerTimers.h>

XRAD_BEGIN

template<class T>
struct	binarization_functor
{
	typedef T result_type;
	typedef T argument_type;

private:
	double	min, max;
	double	divisor;
	result_type	stretch(argument_type x) const { return (x-min)/divisor; }

public:

	binarization_functor(double in_min, double in_max) : min(in_min), max(in_max), divisor(max-min){}

	result_type operator()(result_type &x) const { return range(stretch(x), 0, 1); }

};



class	CFDataPhaseAnalyzer : public S500_CFMFrameSetParams, public PhaseAnalyzerTimers
{
public:
	typedef ComplexFunctionF32 cfm_array_t;
	typedef	DataArray2D<DataArray<cfm_array_t> > shots_array_t;


	virtual double average_frame_offset() const { return 0; }
	virtual wstring message() = 0;// { return L"Processing CFM frames, abstract mode"; }
	virtual void CalculateMask() = 0;

	const FrameAcquisitionBuffer	&stddev() const { return m_stddev;}
	const FrameAcquisitionBuffer	&correlation() const { return m_correlation;}
	const FrameAcquisitionBuffer	&correlation_re_im() const { return m_correlation_re_im; }
	const FrameAcquisitionBuffer	&amplitude() const { return m_amplitude; }

	const FrameAcquisitionBufferStddev	&phase() const { return m_phase; }
	const FrameAcquisitionBuffer	&mask() const { return m_mask; }

	size_t conjugated_count() const { return m_conjugated_count; }

protected:
	FrameAcquisitionBuffer	m_stddev;
	FrameAcquisitionBuffer	m_correlation;
	FrameAcquisitionBuffer	m_correlation_re_im;
	FrameAcquisitionBuffer	m_amplitude;

	FrameAcquisitionBufferStddev	m_phase;
	FrameAcquisitionBuffer	m_mask;

	//! \brief Является ли конкретный луч первым в большом свипе?
	//! Первый луч больших свипов может резко отличаться от остальных. Поэтому вычисления по нему нужно проводить отдельно (или вообще пропустить и не проводить).
	//! Такие ситуации возникают во многих разных алгоритмах
	// Это заплатка для определенных файлов, в которых есть сбой данных, например 
	// Корректность работы проверена на data_store\clinic\Ultrasound\2016_03_Twinkling\2017_09_22_steel+abs\RawBCFCine_22092017_145400.par 
	bool	first_beam_in_large_sweep(size_t beam_no)
	{
		return beam_no && !(beam_no%n_cfm_beams_in_sweep()) && n_cfm_beams_in_sweep() > 2;
	}

private:
	//!		Флаг информирует, делалось ли комплексное сопряжение между отсчетами в пачке. Часть процедур необходимо выполнять только до такого сопряжения
	size_t	m_conjugated_count;

protected:

	// порядок индексов в обрабатываемом массиве

	typedef	MathFunction2D<cfm_array_t> shot_slice_t;

	//!	Данные ЦДК, трехмерный массив в размерности (ray, shot, sample)
	DataArrayMD<shot_slice_t>	cfm_frame;
	//!	Массив двумерных комплексных массивов, представляющий собой ссылки на данные cfm_frames_buffer. Элемент массива -- сигналы с одного "выстрела" в размерности (ray, sample)
	DataArray<shot_slice_t> shot_slices;
	//!	Двумерный массив в координатах (ray, sample) одномерных комплексных массивов, представляющий собой ссылки на данные cfm_frames_buffer. Элементы массива -- доплеровские пачки размером n_shots комплексных чисел
	shots_array_t	shots_array;

	index_vector	frames_buffer_sizes;

	void AddNoise();

	void BlurRFData();
	void ConjugateRFData();
	void ComputePreWallFilterAmplitude();

//		void TissueMotionCompensation();


	void ChangeLastBeam(RealFunction2D_F32 &elastogram);

public:
	void TissueMotionCompensation();
		// критически важные параметры, изменение которых
		// радикально меняет чувствительность за счет разрешающей способности
		// помечены //!!!
		// маловажные параметры, близкие к полному удалению,
		// помечены //---
		// важные параметры, которые могут меняться в зависимости от конфигурации датчика,
		// помечены //+++

	double	frame_agility_factor; //!!!
		// скорость накопления кадров
		// значения от 0 до 1.
		// 1	отсутствие накопления
		// 0.25	умеренное накопление,
		// 0.1	значительное накопление (для малоконтрастных объектов,
		//		оператору не следует при этом слишком быстро менять картинку
		// 0	недопустимо, означает полную потерю данных

//		double	phase_treshold; //---
			//	условное значение фазы, от которого зависит оценка надежности конкретного кадра
//		double nonlinearity_power; //---
			// линейная зависимость накопления от фазы работает слишком "резко", возводим поэтому
			// коэффициент накопления в степень, заданную этим коэффициентом.
			// возможно, этот параметр избыточен (см. комментарий в процедуре).
	double	rf_axial_blur; //+++
		// степень размытия радиочастотных данных.
		// этот параметр найден опытным путем, вряд ли его нужно будет
		// сильно менять. все управление посредством следующей величины

	double	result_axial_blur; //!!!
	double	result_lateral_blur; //!!!
		// степень размытия данных результата (фазовых сдвигов и маски, а также вспомогательных массивов при их расчете).
		//	Латеральное размытие здесь пока не применяется, нужно изучить, целесообразно ли оно
		// (применяется и для комплексного сигнала, и для скалярной эластограммы)

		// Большой блок параметров, объявленных здесь, разнесен по классам-наследникам, где они действительно нужны, 

	//double	result_aspect_ratio; //+++
		// геометрические пропорции входных данных

	CFDataPhaseAnalyzer();

	virtual ~CFDataPhaseAnalyzer() {}

	virtual void	AnalyzeFrame(RealFunction2D_F32 &resulting_data, RealFunction2D_F32 &resulting_mask) = 0;
	virtual cfm_mode algorithm() = 0;

	void ImportOrderedData(const cfm_container_t &current_cfm_frames);
	void ImportRawData(const cfm_container_t::value_type *frames_pointer, bool unsweep);
	virtual void PrepareBuffers(const index_vector &cfm_frames_sizes, size_t in_beams_in_sweep, size_t in_ray_header_size_in_bytes);
	void FlushBuffers();
	virtual void ResetFrameAveraging();
};


XRAD_END

#endif //__PhaseElastoFrameAnalyzer_h