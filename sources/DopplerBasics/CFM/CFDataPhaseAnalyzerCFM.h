#ifndef FrameCFMAnalyzer_h__
#define FrameCFMAnalyzer_h__

/********************************************************************
	created:	2016/10/20
	created:	20:10:2016   14:09
	filename: 	q:\programs\ElastoGrafica\ElastoGrafica\FrameCFMAnalyzer.h
	file path:	q:\programs\ElastoGrafica\ElastoGrafica
	file base:	FrameCFMAnalyzer
	file ext:	h
	author:		kns
	
	purpose:	
*********************************************************************/

#include <DopplerBasics/CFDataPhaseAnalyzer.h>//"CFDataPhaseAnalyzer.h"
#include <DopplerBasics/CFM/WallFilters/WallFilters.h>
#include <DopplerBasics/CFM/WallFilters/WallFiltersInterface.h>
#include <DopplerBasics/CFM/MaskCriteria.h>


XRAD_BEGIN


class CFDataPhaseAnalyzerCFM : public CFDataPhaseAnalyzer
{
	virtual void CalculateMask() override;
	virtual wstring message() override { return L"Processing CFM frames, ColorFlow mode"; }
	virtual void PrepareBuffers(const index_vector& cfm_frames_sizes, size_t in_beams_in_sweep, size_t in_ray_header_size_in_bytes) override;
	virtual void ResetFrameAveraging() override;

public://temporary
	PARENT(CFDataPhaseAnalyzer);

	wall_filter_type	wall_filter_index;
	shared_ptr<WallFilter> wall_filter;


protected:

	struct	phase_normalizer
	{
		const double	amplification = 1.5;
		float operator()(float &y, const float &x){ return y = range((amplification*x+pi())/two_pi(), 0.1, 0.9); };
	};

	void ApplyWallFilter();

	void TissueMotionCompensation();

	void ComputeStandardDeviation();
	void ComputeCorrelationCoefficient();
	void ComputeReImCorrelation();

	double m_tissue_motion_blur;
	bool b_neutralize_acceleration;


	void	ApplyMaskCriteriaSet(FrameAcquisitionBuffer &mask, const mask_criteria_set &mcs);

	FrameAcquisitionBuffer	m_bloodflow_mask;
	FrameAcquisitionBuffer	m_cavitation_mask;
	FrameAcquisitionBuffer	m_oscillation_mask;

public:
	void AcquirePhaseCFM();
	virtual cfm_mode algorithm() override { return cfm_mode::cfm; };
	virtual	 void AnalyzeFrame(RealFunction2D_F32 &result_flow_map, RealFunction2D_F32 &result_mask) override;

	CFDataPhaseAnalyzerCFM()
	{
		// очень важное изменение по сравнению с аналогичными настройками эластографии
		rf_axial_blur = 0;

		m_tissue_motion_blur = 5;
		b_neutralize_acceleration = false;

//		wall_filter_index = wf_LS2;
		wall_filter_index = wf_legendre2;
//		wall_filter_index = no_wf;
//	#pragma message Временно поставлен минимальный wall filter, чтобы посмотреть, есть ли непроизводственные расходы
	}
};

XRAD_END

#endif // FrameCFMAnalyzer_h__
