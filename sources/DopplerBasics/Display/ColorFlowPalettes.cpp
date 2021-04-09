#include "pre.h"
#include <XRADBasic/Sources/Utils/Crayons.h>
#include "ColorFlowPalettes.h"

/********************************************************************
	created:	2016/10/19
	created:	19:10:2016   17:16
	filename: 	q:\programs\ElastoGrafica\sources\ElastoPalettes.cpp
	file path:	q:\programs\ElastoGrafica\sources
	file base:	ElastoPalettes
	file ext:	cpp
	author:		kns
	
	purpose:	
*********************************************************************/

XRAD_BEGIN


GradientPalette	GeneratePaletteCFM(cfm_palette pt)
{
	switch(pt)
	{
		default:
			throw invalid_argument("GeneratePaletteCFM, Unknown palette id");
		case cfm_palette::gray:
		{
			auto palette_points =
			{
				make_pair(0., crayons::black()),
				make_pair(255., crayons::white())
			};
			return 		GradientPalette(palette_points);
		}

		case cfm_palette::cfm_medison:
		{
			auto palette_points =
			{
				make_pair(0., ColorSampleF64(255, 255, 255)),
				make_pair(25., ColorSampleF64(138, 138, 218)),
				make_pair(50., ColorSampleF64(65, 65, 255)),
				make_pair(75., ColorSampleF64(0, 0, 255)),
				make_pair(100., ColorSampleF64(0, 0, 116)),
				make_pair(125., ColorSampleF64(0, 0, 123)),
				make_pair(126., ColorSampleF64(0, 0, 0)),
				//------------------
				make_pair(130., ColorSampleF64(112, 0, 0)),
				make_pair(129., ColorSampleF64(0, 0, 0)),
				make_pair(155., ColorSampleF64(124, 0, 0)),
				make_pair(180., ColorSampleF64(202, 0, 0)),
				make_pair(205., ColorSampleF64(255, 0, 0)),
				make_pair(230., ColorSampleF64(255, 132, 0)),
				make_pair(255., ColorSampleF64(255, 255, 0))
			};

			return 		GradientPalette(palette_points);
		}

		case cfm_palette::cfm_medison_2:
		{
			auto palette_points =
			{
				make_pair(0., ColorSampleF64(0, 240, 125)),
				make_pair(25., ColorSampleF64(51, 208, 222)),
				make_pair(50., ColorSampleF64(103, 148, 238)),
				make_pair(75., ColorSampleF64(91, 88, 194)),
				make_pair(100., ColorSampleF64(64, 49, 192)),
				make_pair(125., ColorSampleF64(53, 26, 204)),
				make_pair(126., ColorSampleF64(0, 0, 0)),
				//------------------
				make_pair(129., ColorSampleF64(0, 0, 0)),
				make_pair(130., ColorSampleF64(190, 27, 21)),
				make_pair(155., ColorSampleF64(174, 42, 24)),
				make_pair(180., ColorSampleF64(226, 81, 51)),
				make_pair(205., ColorSampleF64(236, 116, 74)),
				make_pair(230., ColorSampleF64(255, 166, 111)),
				make_pair(255., ColorSampleF64(255, 173, 113))
			};
			return 		GradientPalette(palette_points);
		}

		case cfm_palette::cfm_sonomed:
		{
			auto palette_points =
			{
				make_pair(0., ColorSampleF64(0, 255, 255)),
				make_pair(25., ColorSampleF64(1, 172, 255)),
				make_pair(50., ColorSampleF64(1, 96, 255)),
				make_pair(75., ColorSampleF64(1, 23, 255)),
				make_pair(100., ColorSampleF64(0, 0, 158)),
				make_pair(125., ColorSampleF64(0, 0, 150)),
				make_pair(126., ColorSampleF64(0, 0, 0)),
				//------------------
				make_pair(129., ColorSampleF64(0, 0, 0)),
				make_pair(130., ColorSampleF64(150, 0, 1)),
				make_pair(155., ColorSampleF64(173, 0, 0)),
				make_pair(180., ColorSampleF64(254, 32, 0)),
				make_pair(205., ColorSampleF64(254, 105, 0)),
				make_pair(230., ColorSampleF64(255, 184, 1)),
				make_pair(255., ColorSampleF64(255, 255, 0))
			};

			return 		GradientPalette(palette_points);
		}

		case cfm_palette::cfm_cascillation:
		{
			auto palette_points =
			{
				make_pair(0., ColorSampleF64(160, 0, 255)),
				make_pair(5., ColorSampleF64(160, 0, 255)),
				
				// violet color for oscillation
				make_pair(5., ColorSampleF64(0, 255, 255)),
				make_pair(10., ColorSampleF64(0, 255, 255)),
				make_pair(25., ColorSampleF64(1, 172, 255)),
				make_pair(50., ColorSampleF64(1, 96, 255)),
				make_pair(75., ColorSampleF64(1, 23, 255)),
				make_pair(100., ColorSampleF64(0, 0, 158)),
				make_pair(125., ColorSampleF64(0, 0, 150)),
				make_pair(126., ColorSampleF64(0, 0, 0)),
				//------------------
				make_pair(129., ColorSampleF64(0, 0, 0)),
				make_pair(130., ColorSampleF64(150, 0, 1)),
				make_pair(155., ColorSampleF64(173, 0, 0)),
				make_pair(180., ColorSampleF64(254, 32, 0)),
				make_pair(205., ColorSampleF64(254, 105, 0)),
				make_pair(230., ColorSampleF64(255, 184, 1)),
				make_pair(245., ColorSampleF64(255, 255, 0)),
				
				// green color for cavitation
				make_pair(250., ColorSampleF64(0, 255, 160)),
				make_pair(255., ColorSampleF64(0, 255, 160))
			};

			return 		GradientPalette(palette_points);
		}


		case cfm_palette::custom:
		{
			auto palette_points =
			{
				make_pair(0., ColorSampleF64(0, 127, 255)),
				make_pair(255., ColorSampleF64(250, 235, 215))
			};
			return 		GradientPalette(palette_points);
		}

		case cfm_palette::elasto_color:
		{
			auto palette_points =
			{
				make_pair(0., crayons::black()),
				make_pair(255., crayons::white())
			};
			return 		GradientPalette(palette_points);
		}
	}
}

XRAD_END
