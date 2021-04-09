#ifndef __elasto_offset_analyzer
#define __elasto_offset_analyzer

#include <XRADBasic/MathFunctionTypes2D.h>
#include <RFDataImport/S500_CFMFrameSet.h>
#include <XRADBasic/Sources/Utils/GradientPalette.h>

XRAD_BEGIN

class	DuplexDisplayer 
	{
	private:

		template<class ARR>
		ColorImageF32	DrawDuplex(
			const ARR& mask_cfm,
			const ARR& slice_cfm,
			GradientPalette& palette,
			ColorScanConverter& SC,
			bool	just_frame
		);


	public:
		// собственные размеры эластограммы
		size_t	window_size_s;// размер эластограммы вдоль луча
		size_t	window_size_r;// размер эластограммы поперек луча
		
		// положение и размеры эластограммы относительно кадра B
		double	cfm_density_r;	// плотность лучей эластограммы относительно плотности лучей B
		double cfm_density_s;	// плотность отсчетов эластограммы относительно плотности отсчетов B
		
		ptrdiff_t window_corner_r; // первый луч B, с которого начинается эластограмма
		ptrdiff_t	window_corner_s; // первый отсчет B, с которого начинается эластограмма
		
		RealInterpolator2D interpolator;
		
	public:
		DuplexDisplayer(const S500_CFMFrameSet &in_fs, const RealFunctionMD_F32 &in_offsets, const RealFunctionMD_F32 &in_mask_frames);
			
		const S500_CFMFrameSet &fs;
		const RealFunctionMD_F32 &cfm_frames;
		const RealFunctionMD_F32 &mask_frames;

			
		void	DisplayDuplex();		
		void	DisplayCFMResults();
	};

//void	AnalyzeElastoFramesOffset(S500_CFMFrameSet &frames);


XRAD_END

#endif //__elasto_offset_analyzer
