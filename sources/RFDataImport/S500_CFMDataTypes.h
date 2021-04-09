#ifndef __S500_CFMDataTypes_h
#define __S500_CFMDataTypes_h

//------------------------------------------------------------------
//
//	created:	2014/06/07
//	created:	7.6.2014   14:33
//	filename: 	Q:\programs\ElastographyTest\sources\ElastoDataTypes.h
//	file path:	Q:\programs\ElastographyTest\sources
//	author:		kns
//	
//	purpose:	
//
//------------------------------------------------------------------

#include <XRADBasic/MathFunctionTypesMD.h>
#include <XRADBasic/MathFunctionTypes2D.h>

XRAD_BEGIN

// объявленные здесь типы должны строго соответствовать
// типам данных, передаваемым библиотечной процедуре из
// вызывающего модуля.
// TODO в объявлении функций вставить фиктивное преобразование
// указателей для того, чтобы исключить несоответствие внутренних
// и внешних типов

typedef	ComplexFunctionF32 b_row_t;
typedef	ComplexFunction2D_F32 b_slice_t;
typedef ComplexFunctionMD_F32 b_container_t;

#if 1
typedef	ComplexFunctionI32F cfm_row_t;
typedef	ComplexFunction2D_I32F cfm_slice_t;
//typedef MathFunctionMD<cfm_slice_t> cfm_container_t;
typedef ComplexFunctionMD_I32F cfm_container_t;
#else
typedef	ComplexFunctionF32 cfm_row_t;
typedef	ComplexFunction2D_F32 cfm_slice_t;
typedef MathFunctionMD<cfm_slice_t> cfm_container_t;
#endif


XRAD_END


#endif //__S500_CFMDataTypes_h