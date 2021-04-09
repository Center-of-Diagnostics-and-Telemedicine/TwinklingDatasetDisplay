#include "pre.h"
#include <XRADBasic/Sources/Utils/StatisticUtils.h>
#include "FrameAcquisitionBuffer.h"

//------------------------------------------------------------------
//
//	created:	2021/01/27	11:07
//	filename: 	FrameAcquisitionBuffer.cpp
//	file path:	q:\Projects\CommonSources\DopplerBasics
//	author:		kns
//	
//	purpose:	
//
//------------------------------------------------------------------

namespace xrad
{

void FrameAcquisitionBuffer::realloc(size_t vs, size_t hs)
{
	m_n_frames_acquired = 0;
	parent::realloc(vs, hs);
	m_memory.realloc(vs, hs);

	flush();
}



void FrameAcquisitionBuffer::resize(size_t vs, size_t hs)
{
	parent::resize(vs, hs);
	m_memory.resize(vs, hs);
	// flush не вызывается, т.к. при resize возможно сохранение части данных
	// По этой же причине не нужен n_frames_acquired = 0;
}



void FrameAcquisitionBuffer::flush()
{
#pragma omp parallel for schedule (guided)
	for(int i = 0; i < int(vsize()); ++i)
	{
		for(int j = 0; j < int(hsize()); ++j)
		{
			at(i, j) = RandomUniformF64();
			m_memory.at(i, j) = RandomUniformF64();
		}
	}
	reset();
}

void FrameAcquisitionBuffer::reset()
{
	fill(0);
	m_memory.fill(0);
	m_n_frames_acquired = 0;
}


//	STDdev buffer

void FrameAcquisitionBuffer::AcquireFrame(double frame_agility_factor, double axial_blur, double lateral_blur)
{
	XRAD_ASSERT_THROW(vsize() == m_memory.vsize() && hsize() == m_memory.hsize());

	if(!m_n_frames_acquired)
	{
		// если данных ранее не поступало, записываем первый кадр как есть
		m_memory.CopyData(*this);
	}
	else
	{
	// иначе применяем рекурсивный фильтр 1-го порядка

		if(frame_agility_factor)
		{
		#pragma omp parallel for schedule (guided)
			for(ptrdiff_t i = 0; i < ptrdiff_t(vsize()); ++i)
			{
				exponential_blur_algorithms::iir_one_point(m_memory.row(i), row(i), -frame_agility_factor);
				ApplyFunction(m_memory.row(i), [](float &x){return is_number(x) ? x: 0;});

				row(i).CopyData(m_memory.row(i));
			}
		}
	}

	double	additional_blur_radius = double(m_n_frames_acquired + 2) / double(m_n_frames_acquired + 1);
	// Пока не накопились данные, изображение может быть сильно зашумлено. Поэтому первые кадры подвергаются дополнительному размытию

	BiexpBlur2D(*this, additional_blur_radius + lateral_blur, additional_blur_radius + axial_blur);
	++m_n_frames_acquired;
}


void FrameAcquisitionBufferStddev::realloc(size_t vs, size_t hs)
{
	m_dispersion.realloc(vs, hs);
	parent::realloc(vs, hs);
}

void FrameAcquisitionBufferStddev::resize(size_t vs, size_t hs)
{
	parent::resize(vs, hs);
	m_dispersion.resize(vs, hs);
}

void FrameAcquisitionBufferStddev::reset()
{
	m_dispersion.fill(0);
	parent::reset();
}

void FrameAcquisitionBufferStddev::flush()
{
#pragma omp parallel for schedule (guided)
	for(int i = 0; i < int(vsize()); ++i)
	{
		for(int j = 0; j < int(hsize()); ++j)
		{
			m_dispersion.at(i, j) = RandomUniformF64();
		}
	}
	parent::flush();
}

void FrameAcquisitionBufferStddev::AcquireFrame(double frame_agility_factor, double axial_blur, double lateral_blur)
{
	XRAD_ASSERT_THROW(vsize() == m_dispersion.vsize() && hsize() == m_dispersion.hsize());

	if(!m_n_frames_acquired)
	{
		// если данных ранее не поступало, СКО не определено
		auto	copy_square = [](float &y, float x)
		{
			return y = square(x);
		};
		m_dispersion.CopyData(*this, copy_square);
	}
	else
	{
		// иначе накапливаем рекурсивно разницу текущего кадра и накопленного среднего
		auto	apply_difference_square = [frame_agility_factor](float &x, float a, float b)
		{
			exponential_blur_algorithms::iir_one_point(x, square(a-b), -frame_agility_factor);
		};
		Apply_AAA_2D_F3(m_dispersion, m_memory, *this, apply_difference_square);
	}

	// Пока не накопились данные, изображение может быть сильно зашумлено. Поэтому первые кадры подвергаются дополнительному размытию
	double	additional_blur_radius = double(m_n_frames_acquired + 2) / double(m_n_frames_acquired + 1);
	BiexpBlur2D(m_dispersion, additional_blur_radius + lateral_blur, additional_blur_radius + axial_blur);

	parent::AcquireFrame(frame_agility_factor, axial_blur, lateral_blur);
}




} //namespace xrad



