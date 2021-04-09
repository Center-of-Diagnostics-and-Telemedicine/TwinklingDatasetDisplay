#ifndef __wall_filter
#define __wall_filter

#include <XRADBasic/LinearVectorTypes.h>
#include <XRADBasic/MathMatrixTypes.h>
#include <XRADBasic/MathFunctionTypesMD.h>


XRAD_BEGIN

//-------------------------------------------------------------------
//	базовый класс с виртуальной функцией
//
struct WallFilter 
{
	enum class update_mode
	{
		none, each_sweep, each_beam, each_frame
	};

	virtual update_mode	GetUpdateMode() { return update_mode::none; }

	virtual	bool is_size_actual(size_t ) const = 0;

	virtual	wstring	filter_name() const = 0;
	virtual void Apply(ComplexFunctionF32 &burst) const = 0;
	virtual void UpdateFilter(ComplexFunctionMD_F32 &/*frames_by_shots*/, size_t /*first_beam*/, size_t /*last_beam*/) {}
	virtual ~WallFilter(){}
};

typedef shared_ptr<WallFilter> WallFilterPtr;

//
//-------------------------------------------------------------------

//	тривиальный фильтр, который ничего не меняет,
//	применяется, когда фильтрация не нужна
struct WallFilterNone : WallFilter
{
	virtual	bool is_size_actual(size_t) const  override { return true; }
	virtual	wstring	filter_name() const override { return L"WallFilterNone"; };
	void Apply(ComplexFunctionF32 &burst) const;// {}
};


// простейший фильтр с вычитанием среднего
struct WallFilterAverage : WallFilter
{
	virtual	bool is_size_actual(size_t) const override { return true; }
	virtual	wstring	filter_name() const override { return L"WallFilterAverage"; };
	void Apply(ComplexFunctionF32 &burst) const {burst -= AverageValue(burst);}
};

// простейший фильтр с нахождением конечных разностей
struct WallFilterFiniteDifference : WallFilter
{
	virtual	bool is_size_actual(size_t) const override { return true; }
	virtual	wstring	filter_name() const override { return L"WallFilterFiniteDifference"; };
	void Apply(ComplexFunctionF32 &burst) const;
};

// полиномиальная регрессия
class WallFilterLS : public WallFilter 
{
	size_t	vector_size;
	size_t	n_components;
	RealFunction2D_F64	polynoms;
	mutable RealVectorF64 re, im;

public:
	virtual	bool is_size_actual(size_t in_vector_size) const override { return vector_size == in_vector_size; }

	void Init(size_t in_vector_size, size_t in_n_components);
	void Apply(ComplexFunctionF32 &burst) const;

	virtual	wstring	filter_name() const override { return ssprintf(L"WallFilterLS, burst size = %zu, n_components = %zu", vector_size, n_components); };
};


class WallFilterBasis: public WallFilter
{
private:
//	ComplexMatrixF64	basis;
	RealMatrixF64	basis;
	size_t	vector_size;
	RealVectorF64	weights;

public:
	WallFilterBasis();
	void Apply(ComplexFunctionF32 &burst) const;
	void InitTrigonometric(size_t vs, const RealVectorF64 &in_weights);
	void InitLegendre(size_t vs, const RealVectorF64 &in_weights);
	void SetMatrix(const RealMatrixF32 &in_m);
	void SetWeights(const RealVectorF64 &in_weights);

	virtual	wstring	filter_name() const override { return L"WallFilterBasis"; };

	virtual	bool is_size_actual(size_t in_vector_size) const override { return vector_size == in_vector_size; }
};




XRAD_END

#endif //__wall_filter