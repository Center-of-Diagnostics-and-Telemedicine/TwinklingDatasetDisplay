#include "pre.h"


#include <RFDataImport/S500_CFMFrameSet.h>
#include <RFDataImport/S500_CFMRawDataDisplay.h>

//#include <DopplerBasics/CFM/WallFilters/WallFiltersInteractive.h>

// тесты универсальных методов доплерографии
//#include "CFMModesDetailedTest.h"

// локальные тесты тонкостей алгоритмов, довольно беспорядочные
//#include "SetOfPulses.h"
#include "DummyHypothesis.h"
#include "SimulateSignal.h"

XRAD_USING;

void	ChooseCFAnalysisTask(S500_CFMFrameSet &frames)
{
	size_t	frame_no = frames.n_frames / 2;
	enum 
	{ 
		cfm_modes_detailed_test, 
		find_blur_1d, 
//		find_blur_2d, 
		set_of_pulses, 
		exit, 
		n_options 
	} option;
	try
	{
		while(true)
		{
			option = GetButtonDecision("Choose CF analysis task",
			{
	//			MakeButton("CFM Modes detailed test",		cfm_modes_detailed_test),
	//			MakeButton("Find Blur 1D", 	find_blur_1d),
	//			MakeButton("Find Blur 2D", 	find_blur_2d),
			//	MakeButton("Set of pulses", set_of_pulses),
				MakeButton("End with this file",			exit)
			});

			switch(option)
			{
	//			case cfm_modes_detailed_test:
	//				CFMModesDetailedTest(frames);
	//				break;
	// 			case find_blur_1d:
	// 				CFMMode_find_blur_1D(frames);
	// 				break;
	// 			case find_blur_2d:
	// 				CFMMode_find_blur_2D(frames);
	// 				break;
				//case set_of_pulses:
					//frame_no = GetUnsigned("CFM-frame to display", MakeGUIValue(frames.n_frames / 2, saved_default_value), 0, frames.n_frames - 1);
					//DisplayASetOfPulses(frames, frame_no);
				//	break;
				case exit:
					throw canceled_operation("Exit button");
					break;
			}
		}
	}
	catch(canceled_operation){}
	catch(quit_application){ throw; }
	catch(...) { Error(GetExceptionString()); }
}



void RawDataProcessingMenu(S500_CFMFrameSet &frames)
{
	try
	{
		enum
		{
			e_display,
			e_analyze, 
			e_exit
		};
		while (true)
		{
			auto answer = Decide(L"Choose option", 
				{ 
				MakeButton(L"Display", e_display),
				MakeButton("End with this file", e_exit)
			});

			switch (answer)
			{
			case e_display:
				DisplayCFMFrameSet(frames, L"CFM frames");
				break;

			default:
				throw canceled_operation("End with this file");
			}
		}
	}
	catch (canceled_operation &) {}
	catch (quit_application &ex) { throw ex; }
	catch(...) { Error(GetExceptionString()); }
}


S500_CFMParamFileData	GetS500FileParams()
{
	wstring	filename = GetFileNameRead(L"Choose data files", saved_default_value, L"*.par");
	S500_CFMParamFileData	params;
	params.Init(filename);
	return params;
}

S500_CFMFrameSet GetS500Frames()
{
	S500_CFMParamFileData	params = GetS500FileParams();
	size_t	start_frame = 0;
	size_t	end_frame = params.NumOfFrames - 1;
	S500_CFMFrameSet	frames(params, start_frame, end_frame);

	frames.ReadAllFrames(true);

	return frames;
}



/*

void	WallFilterFrequencyResponse()
{
	size_t N = GetUnsigned("Burst Size", SavedGUIValue(16), 4, 1024);
	ComplexFunctionF32 pulse(N, complexF32(0));

	while(true)
	{
		pulse[pulse.size() / 2] = complexF32(1);
		ComplexFunctionF32	filtered(pulse);

		WallFilterPtr	wall_filter = GetWallFilterInteractive(pulse.size());
		if(wall_filter->filter_name()==L"WallFilterNone") break;

		wall_filter->Apply(filtered);

		while(true)
		{
			auto	fn = GetButtonDecision(
				L"Display option",
				{
					MakeButton(L"Original pulse", make_fn([&pulse](){DisplayMathFunction(pulse, 0, 1, L"Pulse");})),
					MakeButton(L"WallFilter result",make_fn([&filtered, &wall_filter](){DisplayMathFunction(filtered, 0, 1, L"Filter applied: "+ wall_filter->filter_name());})),
					MakeButton(L"Exit", function<void()>())
				}
			);

			if(fn) fn();
			else break;
		}
	}
}

*/

int xrad::xrad_main(int, char** const)
{
	XRAD_USING

	try
	{
		size_t	answer(0);
		while (true)
		{
			//answer = Decide("Choose source", { "Process raw S500 data", "Process simulated signal", "Wall filter frequency response", "Test dummy hypothesis", "Exit" });
			answer = Decide("Choose source", { "Process raw S500 data", "Process simulated signal", "Test dummy hypothesis", "Exit" });
			switch (answer)
			{
				case 0:
				{
					try
					{
						S500_CFMFrameSet frames = GetS500Frames();
						RawDataProcessingMenu(frames);
					}
					catch(canceled_operation){}
					catch(...){ Error(GetExceptionStringOrRethrow()); }
				}
				break;

				case 1:
				{
					try
					{
						S500_CFMFrameSet frames = SimulateSignal();
						RawDataProcessingMenu(frames);
					}
					catch(canceled_operation){}
					catch(...){ Error(GetExceptionStringOrRethrow()); }
				}
				break;

				case 2:
					DummyHypothesis();
					break;

				default:
					throw canceled_operation("");
			}
		}
	}
	catch (canceled_operation &) {}
	catch (quit_application &) {}
	catch (...) 
	{ 
		Error(GetExceptionString()); 
		return 1;
	}
	return 0;
}
