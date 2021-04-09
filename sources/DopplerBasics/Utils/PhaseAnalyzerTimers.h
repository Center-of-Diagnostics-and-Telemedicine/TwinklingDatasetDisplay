#ifndef __PhaseAnalyzerTimers_h
#define __PhaseAnalyzerTimers_h

#include <XRADBasic/Sources/Utils/TimeProfiler.h>

//------------------------------------------------------------------
//
//	created:	2021/01/27	12:38
//	filename: 	PhaseAnalyzerTimers.h
//	file path:	q:\Projects\CommonSources\DopplerBasics
//	author:		kns
//	
//	purpose:	
//
//------------------------------------------------------------------

namespace xrad
{

class PhaseAnalyzerTimers
{
protected:
	TimeProfiler	tp_variadic;

	TimeProfiler	tp_blur;
	TimeProfiler	tp_postfilter;
	TimeProfiler	tp_conjugate;
	TimeProfiler	tp_acquire;
	TimeProfiler	tp_wall_filter;
	TimeProfiler	tp_build_mask;
public:

	physical_time	GetProcessingTime() const;
	string	GetTimeConsumptionReport() const;
};


}//namespace xrad

#endif //__PhaseAnalyzerTimers_h
