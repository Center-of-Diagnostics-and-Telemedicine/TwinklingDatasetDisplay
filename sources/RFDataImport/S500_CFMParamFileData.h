#ifndef __cfm_param_file_data
#define __cfm_param_file_data

XRAD_BEGIN

struct	S500_CFMParamFileData
{
	wstring	par_filename;
	wstring	dat_filename;

	int	NumOfFrames;
	int	RawFrameSize;
	int	HeaderSize;
	int	NumOfBBeams;
	int	SizeofBBeamAtSamples;
	int	NumOfCFShots;
	int	NumOfSweeps;
	int	BeamsInSweep;
	int	SizeofCFMBeamAtSamples;
	int	FirstScanCFMBeam;
	int	CFMDensity;
	int	NumOfCFMBeams;
	int	NumOfFirstCFMSample;
	int	CFMFilterOrder;
		
	void	Init(const wstring& filename);
};



XRAD_END


#endif //__cfm_param_file_data
