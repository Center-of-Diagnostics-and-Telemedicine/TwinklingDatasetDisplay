#ifndef ElastoPalettes_h__
#define ElastoPalettes_h__

/********************************************************************
	created:	2016/10/19
	created:	19:10:2016   17:16
	filename: 	q:\programs\ElastoGrafica\sources\ElastoPalettes.h
	file path:	q:\programs\ElastoGrafica\sources
	file base:	ElastoPalettes
	file ext:	h
	author:		kns
	
	purpose:	
*********************************************************************/

#include <XRADBasic/Sources/Utils/GradientPalette.h>


XRAD_BEGIN

enum  class cfm_palette
{
	gray,
	cfm_medison,
	cfm_medison_2,
	cfm_sonomed,
	cfm_cascillation,
	elasto_color,
	custom
};

//PaletteBasis	GeneratePaletteCFM(palette_type pt);
//GradientPalette GeneratePaletteCFM(palette_type pt);
GradientPalette GeneratePaletteCFM(cfm_palette pt);


XRAD_END

#endif // ElastoPalettes_h__
