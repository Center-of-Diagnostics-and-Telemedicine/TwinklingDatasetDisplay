#ifndef __pre_h
#define __pre_h

#define __pre_h_first




// это заготовка
#if XRAD_CFG_ARRAYS_INTERACTIONS_VER >= 2
//#include <DataArrayAnalyze.h>
XRAD_BEGIN
#define AA_1D_OpEq Apply_AA_1D_F2
#define AAA_2D_Op Apply_AAA_2D_F3
template <class, class>
using pow_functor = Functors::pow_value;
XRAD_END
#endif


#include <XRADGUI/XRAD.h>
#include <XRADBasic/MathMatrixTypes.h>
#include <XRADBasic/MathFunctionTypesMD.h>
#include <XRADGUI/Sources/GUI/MathFunctionGUI.h>
#include <XRADGUI/Sources/GUI/MathFunctionGUI2D.h>
#include <XRADGUI/Sources/GUI/MathFunctionGUIMD.h> 
#include <XRADBasic/DataArrayIO.h>

#include <XRADGUI/Sources/GUI/Graphset.h>
#include <XRADGUI/Sources/GUI/MatrixVectorGUI.h>

#define RUN_CFM_MODES 1

#undef __pre_h_first

#endif //__pre_h