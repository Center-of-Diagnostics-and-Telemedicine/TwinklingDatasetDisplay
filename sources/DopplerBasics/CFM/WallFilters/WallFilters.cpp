#include "pre.h"

#include <XRADBasic/Sources/Utils/LeastSquares.h>
#include <XRADBasic/Sources/Utils/gram_schmidt.h>

//#include <MatrixAlgorithms/EigenVectorTools.h>

//#include <DopplerBasics/CFM/WallFilters/KarhunenLoeve.h>
#include <DopplerBasics/CFM/WallFilters/WallFilters.h>
//#include <DopplerBasics/CFM/WallFilters/EMD.h>

XRAD_BEGIN


//	тривиальный фильтр, который ничего не меняет,
//	применяется, когда фильтрация не нужна
void WallFilterNone::Apply(ComplexFunctionF32 &burst) const
{

}


//-------------------------------------------------------------------
//
//	простейшие фильтры
//


//	фильтрация с помощью конечных разностей
void WallFilterFiniteDifference::Apply(ComplexFunctionF32 &burst) const
{
	ComplexFunctionF32::iterator it = burst.begin();
	ComplexFunctionF32::iterator it1 = it+1;
	ComplexFunctionF32::iterator ie = burst.end();

	for(;it1<ie; ++it,++it1)
	{
		*it -= *it1;
	}
	*it = 0;
}


//-------------------------------------------------------------------
//
//	разложение по произвольному ортогональному базису
//

WallFilterBasis::WallFilterBasis()
{
}

void WallFilterBasis::Apply(ComplexFunctionF32 &burst) const
{
	for(size_t i = 0; i < burst.size(); ++i)
	{
		double	subtract_factor = 1.-weights[i];
		if(subtract_factor)
		{
			real(burst).subtract_multiply(basis.col(i), sp(basis.col(i), real(burst)));
			imag(burst).subtract_multiply(basis.col(i), sp(basis.col(i), imag(burst)));
		}
	}
}

void WallFilterBasis::InitTrigonometric(size_t vs, const RealVectorF64 &in_weights)
{
	weights.MakeCopy(in_weights);
	vector_size = vs;

	basis.realloc(vector_size, vector_size);
	basis.transpose();//Чтобы отсчеты столбцов матрицы лежали в памяти подряд
	// вектора базиса находятся в столбцах
	for(size_t vector_no = 0; vector_no < vector_size; ++vector_no)
	{
		double	norma(0);
		auto& vec(basis.col(vector_no));

		for(size_t j = 0; j < vector_size; ++j)
		{
			vec[j] = cos(double(vector_no*j)*two_pi()/vector_size);
			norma += square(vec[j]);
		}
		basis.col(vector_no) /= sqrt(norma);
	}
}



void WallFilterBasis::InitLegendre(size_t vs, const RealVectorF64 &in_weights)
{
	//этот базис дает тот же результат, что и полиномиальная регрессия по методу наименьших квадратов, но работает заметно быстрее
	weights.MakeCopy(in_weights);
	vector_size = vs;

	basis.realloc(vector_size, vector_size);
	basis.transpose();//Чтобы отсчеты столбцов матрицы лежали в памяти подряд

	double	vs2 = vector_size/2;
	for(size_t vector_no = 0; vector_no < vector_size; ++vector_no)
	{
		auto& vec(basis.col(vector_no));
		for(size_t j = 0; j < vector_size; ++j)
		{
			double	x = 2.*(double(j) - vs2)/vector_size;
			vec[j] = pow(x, vector_no);
		}
	}
	gso_matrix_columns(basis);
}

void WallFilterBasis::SetMatrix(const RealMatrixF32 &in_m)
{
	vector_size = in_m.hsize();
	basis.MakeCopy(in_m);
}

void WallFilterBasis::SetWeights(const RealVectorF64 &in_weights)
{
	weights.MakeCopy(in_weights);
}



//-------------------------------------------------------------------
//
//	разложение на полиномы по методу наименьших квадратов
//

void WallFilterLS::Apply(ComplexFunctionF32 &burst) const 
{
	DetectLSPolynomUniformGrid(real(burst), re);
	DetectLSPolynomUniformGrid(imag(burst), im);

	for(size_t i = 0; i < n_components; ++i)
	{
		burst.subtract_multiply(polynoms.row(i), complexF64(re[i], im[i]));
	}
}

void WallFilterLS::Init(size_t in_vector_size, size_t in_n_components )
{
	if(in_vector_size<=0 || in_n_components<=0) throw invalid_argument("WallFilterLS::Init(), invalid size");
	vector_size = in_vector_size;
	n_components = in_n_components;

	re.realloc(in_n_components);
	im.realloc(in_n_components);
	
	polynoms.realloc(n_components, vector_size);
	polynoms.row(0).fill(1);
	if(n_components>=2) for(size_t j = 0; j < vector_size; ++j)polynoms.at(1,j) = j;
	if(n_components>=3) for(size_t i = 2; i < n_components; ++i)
	{
		polynoms.row(i).multiply(polynoms.row(i-1), polynoms.row(1));
	}
}

//
//-------------------------------------------------------------------


XRAD_END