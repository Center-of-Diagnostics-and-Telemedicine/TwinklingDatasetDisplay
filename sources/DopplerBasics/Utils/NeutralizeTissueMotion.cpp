#include "pre.h"
#include <XRADBasic/Sources/Utils/ExponentialBlurAlgorithms.h>
#include <XRADBasic/MathMatrixTypes.h>
#include <RFDataImport/S500_CFMFrameSet.h>

#include "NeutralizeTissueMotion.h"


//#include <DopplerBasics/CFM/WallFilters/KarhunenLoeve.h>

XRAD_BEGIN


void NeutralizeUniformMotionOneBeam(CFDataPhaseAnalyzer::shots_array_t &slice, size_t accumulator_blur, size_t n_shots, size_t beam_no)
{
	size_t	n_samples = slice.sizes(1);
	auto &row(slice.row(beam_no));

	ComplexFunctionF32	speed_accumulator(n_samples, complexF32(0));

	for (size_t sample_no = 0; sample_no < n_samples; ++sample_no)
	{
		for (size_t shot_no = 0; shot_no < n_shots - 1; ++shot_no)
		{
			speed_accumulator[sample_no] += complexF32(row.at(sample_no)[shot_no]) % complexF32(row.at(sample_no)[shot_no + 1]);
		}
	
	}
	
	ComplexFunctionF32	phasor(n_samples, complexF32(0));
	for (size_t sample_no = 0; sample_no < n_samples; ++sample_no)
	{
		phasor[sample_no] = polar(1, arg(speed_accumulator[sample_no]));
	}
	ComplexFunction2D_F32	phasors(n_shots, n_samples, complexF32(0));
	phasors.row(0).fill(complexF32(1, 0));
	for (size_t shot_no = 0; shot_no < n_shots - 1; ++shot_no)
	{
		phasors.row(shot_no + 1).multiply(phasors.row(shot_no), phasor);
	}

	for (size_t sample_no = 0; sample_no < n_samples; ++sample_no)
	{
		row.at(sample_no) *= phasors.col(sample_no);
	}
	
}

void NeutralizeAccelerationOneBeam(CFDataPhaseAnalyzer::shots_array_t &slice, size_t accumulator_blur, size_t n_shots, size_t beam_no)
{
	size_t	n_samples = slice.sizes(1);
	auto &row(slice.row(beam_no));

	CFDataPhaseAnalyzer::cfm_array_t speed_accumulator(n_samples, complexF32(0));
	CFDataPhaseAnalyzer::cfm_array_t acceleration_accumulator(n_samples, complexF32(0));
	
	for (size_t sample_no = 0; sample_no < n_samples; ++sample_no)
	{
		CFDataPhaseAnalyzer::cfm_array_t speed(n_shots - 1, complexF32(0));
		for (size_t shot_no = 0; shot_no < n_shots - 1; ++shot_no)
		{
			speed[shot_no] = complexF32(row.at(sample_no)[shot_no]) % complexF32(row.at(sample_no)[shot_no+1]);
		}
		speed_accumulator[sample_no] = ElementSum(speed);

		CFDataPhaseAnalyzer::cfm_array_t acceleration(n_shots - 2, complexF32(0));
		for (size_t shot_no = 0; shot_no < n_shots - 2; ++shot_no)
		{
			acceleration[shot_no] = speed[shot_no] % speed[shot_no + 1];
		}
		for (size_t shot_no = 0; shot_no < n_shots - 2; ++shot_no)
		{
			acceleration_accumulator[sample_no] += acceleration[shot_no];
		}
	}
	acceleration_accumulator.FilterGauss(accumulator_blur);
	RealFunctionF32 acceleration_phase(n_samples);
	for (size_t sample_no = 0; sample_no < n_samples; ++sample_no)
	{
		acceleration_phase[sample_no] = arg(acceleration_accumulator[sample_no]);
	}
	RealFunction2D_F32	acceleration_phases(n_shots, n_samples, 1);
	ComplexFunction2D_F32	acceleration_phasors(n_shots, n_samples);
	for (size_t sample_no = 0; sample_no < n_samples; ++sample_no)
	{
		for (size_t shot_no = 0; shot_no < n_shots; ++shot_no)
		{
			acceleration_phases.at(shot_no, sample_no) = pow(shot_no, 2) * acceleration_phase[sample_no]/2;
			acceleration_phasors.at(shot_no, sample_no) = polar(1, -acceleration_phases.at(shot_no, sample_no));
		}
	}
	for (size_t sample_no = 0; sample_no < n_samples; ++sample_no)
	{
		row.at(sample_no) *= acceleration_phasors.col(sample_no);
	}
}




void NeutralizeTissueMotionOneFrame(CFDataPhaseAnalyzer::shots_array_t  &slice, size_t n_beams_in_sweep, size_t n_shots, double accumulator_blur, bool acceleration_flag)
{
	size_t n_beams = slice.sizes(0);
	size_t n_sweeps = n_beams/n_beams_in_sweep;
	for(size_t sweep_no = 0; sweep_no < n_sweeps; ++sweep_no)
	{
		for (size_t beam_no = 0; beam_no < n_beams_in_sweep; ++beam_no)
		{
			if(acceleration_flag)
			{
				NeutralizeAccelerationOneBeam(slice, accumulator_blur, n_shots, sweep_no*n_beams_in_sweep + beam_no);
			}
			NeutralizeUniformMotionOneBeam(slice, accumulator_blur, n_shots, sweep_no*n_beams_in_sweep+beam_no);
		}
	}
};

XRAD_END