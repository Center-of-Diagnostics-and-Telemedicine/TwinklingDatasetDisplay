#include "pre.h"

#include <RFDataImport/S500_CFMFrameSet.h>
#include <RFDataImport/S500_CFMRawDataDisplay.h>

#include "SimulateSignal.h"

XRAD_BEGIN

S500_CFMFrameSet SimulateSignal()
{
	size_t n_shots = GetUnsigned("Number of shots", 17, 2, 100);
	size_t n_beams = 100;//25;
	size_t n_samples = 300;//50;
	size_t n_sweeps = 1;
	size_t n_frames = 10;
	size_t n_beams_in_sweep = n_beams/n_sweeps;
	size_t n_shots_in_sweep = n_shots*n_beams_in_sweep; 
	size_t n_shots_in_frame = n_shots_in_sweep*n_sweeps;

	ComplexMatrixF32 test_frame_flow(n_samples, n_shots_in_frame, complexF32(0));
	ComplexMatrixF32 test_frame_C_component(test_frame_flow);
	ComplexMatrixF32 test_frame_clutter(test_frame_flow);
	ComplexMatrixF32 test_frame_clutter_accelerated(test_frame_flow);
	ComplexMatrixF32 test_frame_noise(test_frame_flow);
	double amp = decibel_to_amplitude(40);//GetUnsigned("clutter/blood in decibels", 20, 0, 60);
	size_t sigma = 1000;//5000
	ComplexMatrixF32 test_frame0(test_frame_flow);

	S500_CFMParamFileData	test_params;
	test_params.NumOfFrames = int(n_frames);
	test_params.RawFrameSize = 0;
	test_params.HeaderSize = 0;
	test_params.NumOfBBeams = int(n_beams);
	test_params.SizeofBBeamAtSamples = int(n_samples);
	test_params.NumOfCFShots = int(n_shots);
	test_params.NumOfSweeps = int(n_sweeps);
	test_params.BeamsInSweep = int(n_beams_in_sweep);
	test_params.SizeofCFMBeamAtSamples = test_params.SizeofBBeamAtSamples;
	test_params.FirstScanCFMBeam = 0;
	test_params.CFMDensity = 1;
	test_params.NumOfCFMBeams = int(n_beams);
	test_params.NumOfFirstCFMSample = 0;
	test_params.CFMFilterOrder = 0;

	size_t start_frame = 0;
	size_t end_frame = test_params.NumOfFrames - 1;
	S500_CFMFrameSet	test_frames(test_params, start_frame, end_frame);
	test_frames.b_frames.fill(complexF32(0));
	test_frames.cfm_frames.fill(complexF32(1));
	cfm_slice_t	cfm_slice(test_frame_flow);
	double v_c, v_b, v_C, a_c;
	RealFunctionF32 velocity_c(test_params.NumOfFrames), velocity_b(test_params.NumOfFrames), velocity_C_component(n_beams, 0);
	for (size_t n_frame = 0; n_frame < size_t(test_params.NumOfFrames); ++n_frame)
	{
		for (size_t sample = 0; sample < n_samples; ++sample)
		{
			for (size_t shot = 0; shot < n_shots_in_frame; ++shot)
			{
				size_t n_beam = shot / n_shots;
				//double blood_amp = sigma * decibel_to_amplitude(n_beam);
				
				//Моделируются ткани, движущиеся без ускорения
				v_c = (shot*pi() / 10) * (double(n_frame) / (test_params.NumOfFrames - 1));//0.025;
				test_frame_clutter.at(sample, shot) = polar(10000 * amp, v_c);

				//Моделируются ткани, движущиеся с ускорением
				a_c = (shot*shot*pi() / 10) * (double(n_frame) / (test_params.NumOfFrames - 1));//0.025;
				test_frame_clutter_accelerated.at(sample, shot) = polar(10000 * amp, a_c);

				//Моделируется кровоток
				v_b = shot*pi()*(double(n_beam) / (n_beams)); // / 50
				test_frame_flow.at(sample,shot) = polar(10000, v_b);

				//Моделируется компонента упругих колебаний АФП
				size_t pulse = shot - n_beam*n_shots;
				v_C = pulse *  pi();//*2*pi() / n_shots;
				v_C = v_C*n_beam / (n_beams - 1);
				test_frame_C_component.at(sample,shot) = polar(10000, v_C);
				test_frame_C_component.at(sample,shot).im = test_frame_C_component.at(sample,shot).re/2;
				size_t prf = 1000;
				velocity_C_component[n_beam] = prf*v_C/(2*pi()*(n_shots-1));
				//Моделируются шумы
				test_frame_noise.at(sample,shot) = complexF32(RandomGaussian(0, sigma), RandomGaussian(0, sigma));
			}
		}
		test_frame0 = test_frame_clutter;
		//test_frame0 = test_frame_clutter_accelerated;
		//test_frame0 = test_frame_flow;
		//test_frame0 = test_frame_noise;
		//test_frame0 =  test_frame_noise + test_frame_flow;
		//test_frame0 = test_frame_noise + test_frame_C_component;
		//test_frame0 =  test_frame_C_component;
		test_frames.cfm_frames.GetSlice(cfm_slice, { n_frame, slice_mask(1), slice_mask(0) });
		CopyData(cfm_slice, test_frame0);
		if (CapsLock())
		{
			velocity_c[n_frame] = arg(test_frame_clutter.at(n_samples / 2,n_shots_in_frame / 2) % test_frame_clutter.at(n_samples / 2 + 1,n_shots_in_frame / 2 + 1));
			velocity_b[n_frame] = arg(test_frame_flow.at(n_samples / 2,n_shots_in_frame / 2) % test_frame_flow.at(n_samples / 2 + 1,n_shots_in_frame / 2 + 1));
		}
	}
	if (CapsLock())
	{
		DisplayMathFunction(velocity_c, 0, 1, "tissue velocity");
		DisplayMathFunction(velocity_b, 0, 1, "blood velocity");
	}
	//DisplayMathFunction(velocity_C_component, 0, 1, "velocity_C_component");
	return test_frames;
}


XRAD_END