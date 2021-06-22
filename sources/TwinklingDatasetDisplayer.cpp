#include "pre.h"

#include <RFDataImport/S500_CFMFrameSet.h>
#include <RFDataImport/S500_CFMRawDataDisplay.h>

XRAD_USING;



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
				DisplayCFMFrameSet(frames, L"Raw data frames");
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




int xrad::xrad_main(int, char** const)
{
	XRAD_USING

	try
	{
		size_t	answer(0);
		while (true)
		{
			answer = Decide("Choose source", { "Process raw S500 data", "Exit" });
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
