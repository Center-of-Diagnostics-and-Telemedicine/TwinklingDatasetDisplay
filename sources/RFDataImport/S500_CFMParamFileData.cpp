#include "pre.h"
#include <XRADSystem/CFile.h>
#include "S500_CFMParamFileData.h"


XRAD_BEGIN

string	read_line(FILE* file)
{
	string	result;
	char c;
	while(true)
	{
		c = fgetc(file);
		if(c==EOF) throw canceled_operation("");
		if(c != '\n' && c != '\r')
		{
			if(!isspace(c)) result.push_back(c);
		}
		else break;
	}

	while(true)
	{
		c = fgetc(file);
		if(c==EOF) throw canceled_operation("");
		if(c != '\n' || c != '\r') break;
	}
	fseek(file, -1, SEEK_CUR);

	return result;
}

vector<string> GetStrings(FILE *file)
{
	vector<string> result;
	try
	{
		while(true)
		{
			result.push_back(read_line(file));
		}
	}
	catch(const canceled_operation&) {}

	return result;
}

int	GetParam(const string &key, const vector<string>& v, int default_value = 0)
{
	int	result = 0;
	for(auto& ln: v)
	{
		auto count = 	sscanf(ln.c_str(), key.c_str(), &result);
		if(count) return result;
	}
	return default_value;
}

void	S500_CFMParamFileData::Init(const wstring &filename)
{
	wchar_t	par_fn_buffer[512];
	wchar_t	dat_fn_buffer[512];

	wcscpy(dat_fn_buffer, filename.c_str());
	wcscpy(par_fn_buffer, filename.c_str());

	size_t	n = wcslen(dat_fn_buffer)-4;

	wcscpy(dat_fn_buffer+n, L".dat");
	wcscpy(par_fn_buffer+n, L".par");

	dat_filename = wstring(dat_fn_buffer);
	par_filename = wstring(par_fn_buffer);

	shared_cfile par_file;
	par_file.open(par_filename.c_str(), L"rb");

	auto file_strings = GetStrings(par_file.c_file());

	NumOfFrames = GetParam("NumOfFrames=%d", file_strings);
	RawFrameSize = GetParam("RawFrameSize=%d", file_strings);
	HeaderSize = GetParam("HeaderSize=%d", file_strings);
	NumOfBBeams = GetParam("NumOfBBeams=%d", file_strings);
	SizeofBBeamAtSamples = GetParam("SizeofBBeamAtSamples=%d", file_strings);
	NumOfCFShots = GetParam("NumOfCFShots=%d", file_strings);
	NumOfSweeps = GetParam("NumOfSweeps=%d", file_strings);
	BeamsInSweep = GetParam("BeamsInSweep=%d", file_strings);
	SizeofCFMBeamAtSamples = GetParam("SizeofCFMBeamAtSamples=%d", file_strings);
	FirstScanCFMBeam = GetParam("FirstScanCFMBeam=%d", file_strings);
	CFMDensity = GetParam("CFMDensity=%d", file_strings);
	NumOfCFMBeams = GetParam("NumOfCFMBeams=%d", file_strings);
	NumOfFirstCFMSample = GetParam("NumOfFirstCFMSample=%d", file_strings);
	CFMFilterOrder = GetParam("CFMFilterOrder=%d", file_strings);
}



XRAD_END


