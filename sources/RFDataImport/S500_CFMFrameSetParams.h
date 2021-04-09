#ifndef __CFMFrameSetParams_h
#define __CFMFrameSetParams_h

//------------------------------------------------------------------
//
//	created:	2014/06/10
//	created:	10.6.2014   10:27
//	filename: 	Q:\programs\ElastographyTest\sources\CFMFrameSetParams.h
//	file path:	Q:\programs\ElastographyTest\sources
//	author:		kns
//	
//	purpose:	
//
//------------------------------------------------------------------

#include "S500_CFMParamFileData.h"


XRAD_BEGIN

struct S500_CFMFrameSetParams
{
	enum
	{
		ray_index = 0,
		shot_index = 1,
		sample_index = 2
	};

	S500_CFMFrameSetParams(const S500_CFMParamFileData &in_params):
		m_n_cfm_beams(in_params.NumOfCFMBeams),
		m_n_cfm_samples(in_params.SizeofCFMBeamAtSamples),
		m_n_cfm_shots(in_params.NumOfCFShots),
		m_n_cfm_beams_in_sweep(in_params.BeamsInSweep),
		m_n_cfm_sweeps(in_params.NumOfSweeps),
		m_ray_header_size_in_bytes(in_params.HeaderSize)
	{}

	S500_CFMFrameSetParams() :
		m_n_cfm_beams(0),
		m_n_cfm_samples(0),
		m_n_cfm_shots(0),
		m_n_cfm_beams_in_sweep(0),
		m_n_cfm_sweeps(0),
		m_ray_header_size_in_bytes(0)
	{}


	void	SetCFMFrameSizes(const index_vector &new_frames_buffer_sizes, size_t in_beams_in_sweep, size_t in_ray_header_size_in_bytes)
	{
		m_n_cfm_beams_in_sweep = in_beams_in_sweep;
		m_n_cfm_beams = new_frames_buffer_sizes[ray_index];
		m_n_cfm_shots = new_frames_buffer_sizes[shot_index];
		m_n_cfm_samples = new_frames_buffer_sizes[sample_index];

		m_n_cfm_sweeps = m_n_cfm_beams_in_sweep ? m_n_cfm_beams/ m_n_cfm_beams_in_sweep : 0;

		m_ray_header_size_in_bytes = in_ray_header_size_in_bytes;
	}


	size_t	n_cfm_beams() const				{return m_n_cfm_beams;}
	size_t	n_cfm_samples() const			{return m_n_cfm_samples;}
	size_t	n_cfm_shots() const				{return m_n_cfm_shots;}
	size_t	n_cfm_beams_in_sweep() const	{return m_n_cfm_beams_in_sweep;}
	size_t	n_cfm_sweeps() const			{return m_n_cfm_sweeps;}
	size_t	ray_header_size_in_bytes() const {return m_ray_header_size_in_bytes;}


	size_t	ReorderRays(size_t source_row_no, bool unsweep)
	{
		size_t	pack_size = n_cfm_beams_in_sweep() * n_cfm_shots();
		size_t	row_in_pack = source_row_no%pack_size;
		size_t	pack_start = source_row_no - row_in_pack;

		size_t	m1 = row_in_pack/n_cfm_beams_in_sweep();
		size_t	m2 = row_in_pack%n_cfm_beams_in_sweep();

		if(unsweep)
		{
			return pack_start + m1 + m2*n_cfm_shots();
			// для разбора свипов по пачкам
		}
		else
		{
			return pack_start + row_in_pack;
			// для чтения кадра подряд
		}
	}

private:
	size_t	m_n_cfm_beams;
	size_t	m_n_cfm_samples;
	size_t	m_n_cfm_shots;
	size_t	m_n_cfm_beams_in_sweep;
	size_t	m_n_cfm_sweeps;
	size_t	m_ray_header_size_in_bytes;

};

XRAD_END


#endif //__CFMFrameSetParams_h