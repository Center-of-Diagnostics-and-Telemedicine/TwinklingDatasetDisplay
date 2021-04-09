#include "pre.h"
#include "DummyHypothesis.h"

#include <XRADBasic/Sources/Utils/LeastSquares.h> //введена для dummy

XRAD_BEGIN


ComplexFunctionF32	circle(double y, double x, double ry, double rx)
{
	size_t	n = 16;
	size_t	levels = 15;

	ComplexFunctionF32	c(n*levels);
	for(size_t l = 0; l < levels; ++l)
	{
		for(size_t i = 0; i < n; ++i)
		{
			auto cc = polar(1, two_pi()*double(i) / (n - 1));
			real(cc) *= ry*(levels - l) / levels;
			imag(cc) *= rx*(levels - l) / levels;
			c[i + l*n] = complexF32(y, x) + cc;
		}
	}
	return c;
}

ComplexFunctionF32	star(double y, double x, double ry, double rx)
{
	size_t	n = 16;
	size_t	levels = 15;

	ComplexFunctionF32	c(n*levels);

	for(size_t i = 0; i < n; ++i)
	{
		double	factor = 1. + (i % 2);
		auto cc = polar(factor * RandomUniformF64(0.5, 1), two_pi()*double(i) / n);
		real(cc) *= ry;
		imag(cc) *= rx;
		c[i] = complexF32(y, x) + cc;
		//c[i] = cc;
	}
	c[n - 1] = c[0];

	for(size_t l = 0; l < levels; ++l)
	{
		for(size_t i = 0; i < n; ++i)
		{
			c[i + l*n] = complexF32(y, x) + (c[i] - complexF32(y, x)) * (levels - l) / levels;
		}
	}
	return c;
}

void	ConvolveBurst(const ComplexFunctionF32 &pulse)
{
	int	N(5000);
	ComplexFunctionF32	scatterers(N), signal(N, complexF32(0));
	for(auto &x : scatterers)
	{
		x = polar(1, RandomUniformF64(-pi(), pi()));
	}

	scatterers[N / 2] *= 10000;

	for(int i = 0; i < int(signal.size()); ++i)
	{
		for(int j = 0; j < int(pulse.size()); ++j)
		{
			int	sample = i - (j - int(pulse.size()) / 2);
			if(in_range(sample, 0, N - 1))
			{
				signal[i] += pulse[j] * scatterers[sample];
			}
		}
	}
	//DisplayMathFunction(scatterers, 0, 1, "scatterers");
	//DisplayMathFunction(signal, 0, 1, "Signal");
	//DisplayMathFunction(pulse, 0, 1, "pulse");
}


void DummyHypothesis()
{
	size_t	n_steps = 32;
	ptrdiff_t step = -1;
	size_t	answer(0);
	while(answer != 3)
	{
		step += 1;
		static GraphSet	B_dot(L"", L"Значение", L"Позиция");
		static GraphSet	A_dot(L"", L"Значение", L"Позиция");
		static GraphSet	C_dot(L"", L"Значение", L"Позиция");
		static GraphSet	D_dot(L"", L"Значение", L"Позиция");
		static GraphSet	processes_dots(L"", L"amplitude", L"value");
		static GraphSet	B_Complex_plane(L"", L"Мнимая", L"Действительная");
		static GraphSet	A_Complex_plane(L"", L"Мнимая", L"Действительная");
		static GraphSet	C_Complex_plane(L"", L"Мнимая", L"Действительная");
		static GraphSet	D_Complex_plane(L"", L"Мнимая", L"Действительная");
		static GraphSet	signals_Complex_plane(L"", L"real", L"imag");
		static GraphSet	B_Cartesian_plane(L"", L"Значение", L"Номер импульса");
		static GraphSet	A_Cartesian_plane(L"", L"Значение", L"Номер импульса");
		static GraphSet	C_Cartesian_plane(L"", L"Значение", L"Номер импульса");
		static GraphSet	D_Cartesian_plane(L"", L"Значение", L"Номер импульса");
		static GraphSet	signals_Cartesian_plane(L"", L"amplitude", L"t");
		const double	x0 = -pi();
		static double x_blood(x0);
		static double x_oscillating_obj(x0);
		static double x_stationary_obj(x0);//1.25*pi());
		//x_oscillating_obj = x0 + 0.25*sin(2.33 * x_blood);
		int n_steps_per_period = 12;
		x_oscillating_obj = x0 + 0.25*sin(n_steps*x_blood / n_steps_per_period);
		//x_calc = x_blood;

		double	ry(0.04), rx(0.8);
		//double	ry(0.01), rx(0.025);

		ComplexFunctionF32 burst(314);
		ComplexFunctionF32 point_blood = circle(0.5, x_blood, ry, rx);//circle(0.75, x_blood, ry, rx);
		ComplexFunctionF32 point_stationary_obj = circle(0.5, x_stationary_obj, ry, rx);
		ComplexFunctionF32 point_oscillating = circle(0.5, x_oscillating_obj, ry, rx);
		ComplexFunctionF32 point_cavitating = star(0.5, x0, 2 * ry, 2 * rx);

		ComplexFunctionF32 vector_blood(2, complexF32(0));
		ComplexFunctionF32 vector_oscillation(2, complexF32(0));
		ComplexFunctionF32 vector_cavitation(2, complexF32(0));
		ComplexFunctionF32 vector_stationary_obj(2, complexF32(0));

		static vector<complexF32> vpath_blood;
		static vector<complexF32> vpath_oscillation;
		static vector<complexF32> vpath_cavitation;
		static vector<complexF32> vpath_stationary_obj;


		double sample_rate = two_pi() * 5. / burst.size();
		double initial_phase_shift = -sample_rate*burst.size() / 2;

		for(size_t i = 0; i < burst.size(); ++i)
		{
			double	phase = sample_rate*i + initial_phase_shift;
			double amplitude = gauss(phase, two_pi());
			real(burst[i]) = amplitude * cos(phase);
			imag(burst[i]) = amplitude * sin(phase);
			if(phase <= x_blood)
			{
				vector_blood[1] = burst[i];
			}
			if(phase <= x_oscillating_obj)
			{
				vector_oscillation[1] = burst[i];
			}
			vector_cavitation[1] = complexF32(RandomUniformF64(-1, 1), RandomUniformF64(-1, 1));
			if(phase <= x_stationary_obj) vector_stationary_obj[1] = burst[i];
		}

		do_once ConvolveBurst(burst);

		vpath_blood.push_back(vector_blood[1]);
		vpath_oscillation.push_back(vector_oscillation[1]);
		vpath_cavitation.push_back(vector_cavitation[1]);
		vpath_stationary_obj.push_back(vector_stationary_obj[1]);

		ComplexFunctionF32 path_blood(vpath_blood.begin(), vpath_blood.end());
		ComplexFunctionF32 path_oscillation(vpath_oscillation.begin(), vpath_oscillation.end());
		ComplexFunctionF32 path_cavitation(vpath_cavitation.begin(), vpath_cavitation.end());
		ComplexFunctionF32 path_stationary_obj(vpath_stationary_obj.begin(), vpath_stationary_obj.end());

		do_once
		{
			RealFunctionF32	dummy(1, 0);
			processes_dots.AddGraphUniform(dummy, 0, 0.0001, L"");//4
			processes_dots.AddGraphUniform(dummy, 0, 0.0001, L"");//5
			processes_dots.AddGraphUniform(dummy, 0, 0.0001, L"");//6
			processes_dots.AddGraphUniform(dummy, 0, 0.0001, L"");//7

			RealFunctionF32 envelop(314);
			for(size_t i = 0; i < burst.size(); i++)
			{
				envelop[i] = sqrt(real(burst[i])*real(burst[i]) + imag(burst[i])*imag(burst[i]));
			}
			B_dot.AddGraphParametric(real(point_stationary_obj), imag(point_stationary_obj), L"Неподвижный объект");
			B_dot.AddGraphUniform(envelop, initial_phase_shift, sample_rate, L"Огибающая");
			B_dot.SetScale(range2_F64(0, -15, 1.5, 15));
			graph_line_style style(dashed_black_lines);
			double line_width(5.5);
			B_dot.SetGraphStyle(style, line_width);

			A_dot.AddGraphParametric(real(point_blood), imag(point_blood), L"Подвижный объект");
			A_dot.AddGraphUniform(envelop, initial_phase_shift, sample_rate, L"Огибающая");
			A_dot.SetScale(range2_F64(0, -15, 1.5, 15));
			A_dot.SetGraphStyle(style, line_width);

			C_dot.AddGraphParametric(real(point_oscillating), imag(point_oscillating), L"Подвижный объект");
			C_dot.AddGraphUniform(envelop, initial_phase_shift, sample_rate, L"Огибающая");
			C_dot.SetScale(range2_F64(0, -15, 1.5, 15));
			//C_dot.SetScale(range2_F64(0.488, -3.45, 0.512, -2.85)); //для визуального увеличения области перемещения точки
			C_dot.SetGraphStyle(style, line_width);

			D_dot.AddGraphParametric(real(point_cavitating), imag(point_cavitating), L"Кавитация");
			D_dot.AddGraphUniform(envelop, initial_phase_shift, sample_rate, L"Огибающая");
			D_dot.SetScale(range2_F64(0, -15, 1.5, 15));
			D_dot.SetGraphStyle(style, line_width);

			processes_dots.AddGraphParametric(real(point_blood), imag(point_blood), L"Blood flow");
			processes_dots.AddGraphParametric(real(point_oscillating), imag(point_oscillating), L"Oscillating object");
			processes_dots.AddGraphParametric(real(point_cavitating), imag(point_cavitating), L"Cavitating object");
			processes_dots.AddGraphParametric(real(point_stationary_obj), imag(point_stationary_obj), L"Stationary object");

			processes_dots.AddGraphUniform(real(burst), initial_phase_shift, sample_rate, L"Re burst");
			processes_dots.AddGraphUniform(imag(burst), initial_phase_shift, sample_rate, L"Im burst");
			processes_dots.SetScale(range2_F64(-1, -15, 1, 15));

			B_Complex_plane.AddGraphParametric(real(vector_stationary_obj), imag(vector_stationary_obj), L"Вектор");
			B_Complex_plane.AddGraphParametric(real(path_stationary_obj), imag(path_stationary_obj), L"Путь");
			B_Complex_plane.SetScale(range2_F64(-1, -1, 1, 1));
			B_Complex_plane.SetGraphStyle(style, line_width);

			//A_Complex_plane.AddGraphParametric(real(vector_blood), imag(vector_blood), L"Вектор");
			A_Complex_plane.AddGraphParametric(real(path_blood), imag(path_blood), L"Путь");
			A_Complex_plane.SetScale(range2_F64(-1, -1, 1, 1));
			A_Complex_plane.SetGraphStyle(style, line_width);

			//C_Complex_plane.AddGraphParametric(real(vector_oscillation), imag(vector_oscillation), L"Вектор");
			C_Complex_plane.AddGraphParametric(real(path_oscillation), imag(path_oscillation), L"Путь");
			C_Complex_plane.SetScale(range2_F64(-1, -1, 1, 1));
			C_Complex_plane.SetGraphStyle(style, line_width);

			D_Complex_plane.AddGraphParametric(real(vector_cavitation), imag(vector_cavitation), L"Вектор");
			D_Complex_plane.AddGraphParametric(real(path_cavitation), imag(path_cavitation), L"Путь");
			D_Complex_plane.SetScale(range2_F64(-1, -1, 1, 1));
			D_Complex_plane.SetGraphStyle(style, line_width);

			signals_Complex_plane.AddGraphParametric(real(vector_blood), imag(vector_blood), L"Blood flow vector");
			signals_Complex_plane.AddGraphParametric(real(vector_oscillation), imag(vector_oscillation), L"Oscillation vector");
			signals_Complex_plane.AddGraphParametric(real(vector_cavitation), imag(vector_cavitation), L"Cavitation vector");
			signals_Complex_plane.AddGraphParametric(real(vector_stationary_obj), imag(vector_stationary_obj), L"Stationary vector");
			signals_Complex_plane.AddGraphParametric(real(path_blood), imag(path_blood), L"Blood flow path");
			signals_Complex_plane.AddGraphParametric(real(path_oscillation), imag(path_oscillation), L"Oscillation path");
			signals_Complex_plane.AddGraphParametric(real(path_cavitation), imag(path_cavitation), L"Cavitation path");
			signals_Complex_plane.AddGraphParametric(real(path_stationary_obj), imag(path_stationary_obj), L"Stationary path");
			signals_Complex_plane.SetScale(range2_F64(-1, -1, 1, 1));

			B_Cartesian_plane.AddGraphUniform(imag(path_stationary_obj), 1, 0, L"Действительная часть");
			B_Cartesian_plane.AddGraphUniform(real(path_stationary_obj), 1, 0, L"Мнимая часть");
			B_Cartesian_plane.SetScale(range2_F64(-1, 0, 1, 15 * pi()));
			B_Cartesian_plane.SetGraphStyle(style, line_width);

			A_Cartesian_plane.AddGraphUniform(imag(path_blood), 1, 0, L"Действительная часть");
			A_Cartesian_plane.AddGraphUniform(real(path_blood), 1, 0, L"Мнимая часть");
			A_Cartesian_plane.SetScale(range2_F64(-1, 0, 1, 15 * pi()));
			A_Cartesian_plane.SetGraphStyle(style, line_width);

			C_Cartesian_plane.AddGraphUniform(imag(path_oscillation), 1, 0, L"Действительная часть");
			C_Cartesian_plane.AddGraphUniform(real(path_oscillation), 1, 0, L"Мнимая часть");
			C_Cartesian_plane.SetScale(range2_F64(-1, 0, 1, 15 * pi()));
			C_Cartesian_plane.SetGraphStyle(style, line_width);

			D_Cartesian_plane.AddGraphUniform(imag(path_cavitation), 1, 0, L"Действительная часть");
			D_Cartesian_plane.AddGraphUniform(real(path_cavitation), 1, 0, L"Мнимая часть");
			D_Cartesian_plane.SetScale(range2_F64(-1, 0, 1, 15 * pi()));
			D_Cartesian_plane.SetGraphStyle(style, line_width);

			signals_Cartesian_plane.AddGraphUniform(real(path_blood), 1, 0, L"Re blood");
			signals_Cartesian_plane.AddGraphUniform(real(path_oscillation), 1, 0, L"Re calc");
			signals_Cartesian_plane.AddGraphUniform(real(path_cavitation), 1, 0, L"Re cavi");
			signals_Cartesian_plane.AddGraphUniform(real(path_stationary_obj), 1, 0, L"Re stationary obj");
			signals_Cartesian_plane.AddGraphUniform(imag(path_blood), 1, 0, L"Im blood");
			signals_Cartesian_plane.AddGraphUniform(imag(path_oscillation), 1, 0, L"Im calc");
			signals_Cartesian_plane.AddGraphUniform(imag(path_cavitation), 1, 0, L"Im cavi");
			signals_Cartesian_plane.AddGraphUniform(imag(path_stationary_obj), 1, 0, L"Im stationary obj");
			signals_Cartesian_plane.SetScale(range2_F64(-1, 0, 1, 20 * pi()));
		}
		A_dot.ChangeGraphParametric(0, real(point_blood), imag(point_blood), L"Подвижный объект");
		C_dot.ChangeGraphParametric(0, real(point_oscillating), imag(point_oscillating), L"Подвижный объект");
		D_dot.ChangeGraphParametric(0, real(point_cavitating), imag(point_cavitating), L"Кавитация");

		processes_dots.ChangeGraphParametric(4, real(point_blood), imag(point_blood), L"Blood flow");
		processes_dots.ChangeGraphParametric(5, real(point_oscillating), imag(point_oscillating), L"Oscillating object");
		processes_dots.ChangeGraphParametric(6, real(point_cavitating), imag(point_cavitating), L"Cavitating object");
		processes_dots.ChangeGraphParametric(7, real(point_stationary_obj), imag(point_stationary_obj), L"Stationary object");
		processes_dots.ChangeGraphUniform(8, real(burst), initial_phase_shift, sample_rate, L"Re burst");
		processes_dots.ChangeGraphUniform(9, imag(burst), initial_phase_shift, sample_rate, L"Im burst");

		B_Complex_plane.ChangeGraphParametric(0, real(vector_stationary_obj), imag(vector_stationary_obj), L"Вектор");
		B_Complex_plane.ChangeGraphParametric(1, real(path_stationary_obj), imag(path_stationary_obj), L"Путь");
		//A_Complex_plane.ChangeGraphParametric(0, real(vector_blood), imag(vector_blood), L"Вектор");
		A_Complex_plane.ChangeGraphParametric(1, real(path_blood), imag(path_blood), L"Путь");
		//C_Complex_plane.ChangeGraphParametric(0, real(vector_oscillation), imag(vector_oscillation), L"Вектор");
		C_Complex_plane.ChangeGraphParametric(1, real(path_oscillation), imag(path_oscillation), L"Путь");
		D_Complex_plane.ChangeGraphParametric(0, real(vector_cavitation), imag(vector_cavitation), L"Вектор");
		D_Complex_plane.ChangeGraphParametric(1, real(path_cavitation), imag(path_cavitation), L"Путь");

		signals_Complex_plane.ChangeGraphParametric(0, real(vector_blood), imag(vector_blood), L"Blood flow vector");
		signals_Complex_plane.ChangeGraphParametric(1, real(vector_oscillation), imag(vector_oscillation), L"Oscillating vector");
		signals_Complex_plane.ChangeGraphParametric(2, real(vector_cavitation), imag(vector_cavitation), L"Cavitation vector");
		signals_Complex_plane.ChangeGraphParametric(3, real(vector_stationary_obj), imag(vector_stationary_obj), L"Stationary vector");
		signals_Complex_plane.ChangeGraphParametric(4, real(path_blood), imag(path_blood), L"Blood flow path");
		signals_Complex_plane.ChangeGraphParametric(5, real(path_oscillation), imag(path_oscillation), L"Oscillating flow path");
		signals_Complex_plane.ChangeGraphParametric(6, real(path_cavitation), imag(path_cavitation), L"Cavitation path");
		signals_Complex_plane.ChangeGraphParametric(7, real(path_stationary_obj), imag(path_stationary_obj), L"Stationary path");

		B_Cartesian_plane.ChangeGraphUniform(0, imag(path_stationary_obj), 0, 1, L"Действительная часть");
		B_Cartesian_plane.ChangeGraphUniform(1, real(path_stationary_obj), 0, 1, L"Мнимая часть");
		A_Cartesian_plane.ChangeGraphUniform(0, imag(path_blood), 0, 1, L"Действительная часть");
		A_Cartesian_plane.ChangeGraphUniform(1, real(path_blood), 0, 1, L"Мнимая часть");
		C_Cartesian_plane.ChangeGraphUniform(0, imag(path_oscillation), 0, 1, L"Действительная часть");
		C_Cartesian_plane.ChangeGraphUniform(1, real(path_oscillation), 0, 1, L"Мнимая часть");
		D_Cartesian_plane.ChangeGraphUniform(0, imag(path_cavitation), 0, 1, L"Действительная часть");
		D_Cartesian_plane.ChangeGraphUniform(1, real(path_cavitation), 0, 1, L"Мнимая часть");

		signals_Cartesian_plane.ChangeGraphUniform(0, real(path_blood), 0, 1, L"Re blood");
		signals_Cartesian_plane.ChangeGraphUniform(1, real(path_oscillation), 0, 1, L"Re calc");
		signals_Cartesian_plane.ChangeGraphUniform(2, real(path_cavitation), 0, 1, L"Re cavi");
		signals_Cartesian_plane.ChangeGraphUniform(3, real(path_stationary_obj), 0, 1, L"Re stationary");
		signals_Cartesian_plane.ChangeGraphUniform(4, imag(path_blood), 0, 1, L"Im blood");
		signals_Cartesian_plane.ChangeGraphUniform(5, imag(path_oscillation), 0, 1, L"Im calc");
		signals_Cartesian_plane.ChangeGraphUniform(6, imag(path_cavitation), 0, 1, L"Im cavi");
		signals_Cartesian_plane.ChangeGraphUniform(7, imag(path_stationary_obj), 0, 1, L"Im stationary");

		x_blood += two_pi() / n_steps;
		if(x_blood > x0 + 3 * pi())
		{
			x_blood = x0;
			vpath_blood.clear();
			vpath_oscillation.clear();
			vpath_cavitation.clear();
			vpath_stationary_obj.clear();
		}
		//signals_Complex_plane.Display(false);
		//signals_Cartesian_plane.Display(false);
		//processes_dots.Display(false);
		//B_dot.Display(false);
		//A_dot.Display();
		//C_dot.Display();
		D_dot.Display(false);
		//B_Complex_plane.Display();
		//A_Complex_plane.Display();
		//C_Complex_plane.Display();
		D_Complex_plane.Display(false);
		//B_Cartesian_plane.Display(false);
		//A_Cartesian_plane.Display(false);
		//C_Cartesian_plane.Display();
		D_Cartesian_plane.Display();
	}
}

void dummy()
{
	size_t	answer(0);
	while(answer != 3)
	{
		static GraphSet	processes_dots(L"", L"amplitude", L"value");
		static GraphSet	signals_Complex_plane(L"", L"real", L"imag");
		static GraphSet	signals_Cartesian_plane(L"", L"amplitude", L"t");
		const double	x0 = -pi();
		static double x_blood(x0);
		static double x_calc(0);
		static double x_stationary_obj(1.25*pi());

		x_calc = pi() / 4 + 0.25*sin(2.33 * x_blood);

		double	ry(0.01), rx(0.1);

		ComplexFunctionF32 burst(314);
		ComplexFunctionF32 point_blood = circle(0.75, x_blood, ry, rx);
		ComplexFunctionF32 point_stationary_obj = circle(0.5, x_stationary_obj, ry, rx);
		ComplexFunctionF32 point_calc = circle(0.25, x_calc, ry, rx);
		ComplexFunctionF32 point_cavi = star(-0.5, 0, 2 * ry, 2 * rx);

		ComplexFunctionF32 vector_blood(2, complexF32(0));
		ComplexFunctionF32 vector_calc(2, complexF32(0));
		ComplexFunctionF32 vector_cavi(2, complexF32(0));
		ComplexFunctionF32 vector_stationary_obj(2, complexF32(0));

		static vector<complexF32> vpath_blood;
		static vector<complexF32> vpath_calc;
		static vector<complexF32> vpath_cavi;
		static vector<complexF32> vpath_stationary_obj;


		double sample_rate = two_pi() * 5. / burst.size();
		double initial_phase_shift = -sample_rate*burst.size() / 2;

		for(size_t i = 0; i < burst.size(); ++i)
		{
			double	t = sample_rate*i + initial_phase_shift;
			double amplitude = gauss(t, two_pi());
			real(burst[i]) = (amplitude + RandomUniformF64(-0.02, 0.02))* cos(t);
			imag(burst[i]) = (amplitude + RandomUniformF64(-0.02, 0.02)) * sin(t);
			if(t <= x_blood) vector_blood[1] = burst[i];
			if(t <= x_calc) vector_calc[1] = burst[i];
			vector_cavi[1] = complexF32(RandomUniformF64(-0.25, 0.25), RandomUniformF64(-0.25, 0.25));
			if(t <= x_stationary_obj) vector_stationary_obj[1] = burst[i];
		}

		do_once ConvolveBurst(burst);

		vpath_blood.push_back(vector_blood[1]);
		vpath_calc.push_back(vector_calc[1]);
		vpath_cavi.push_back(vector_cavi[1]);
		vpath_stationary_obj.push_back(vector_stationary_obj[1]);

		ComplexFunctionF32 path_blood(vpath_blood.begin(), vpath_blood.end());
		ComplexFunctionF32 path_calc(vpath_calc.begin(), vpath_calc.end());
		ComplexFunctionF32 path_cavi(vpath_cavi.begin(), vpath_cavi.end());
		ComplexFunctionF32 path_stationary_obj(vpath_stationary_obj.begin(), vpath_stationary_obj.end());

		do_once
		{
			RealFunctionF32	dummy(1, 0);
			processes_dots.AddGraphUniform(dummy, 0, 0.0001, L"");//4
			processes_dots.AddGraphUniform(dummy, 0, 0.0001, L"");//5
			processes_dots.AddGraphUniform(dummy, 0, 0.0001, L"");//6
			processes_dots.AddGraphUniform(dummy, 0, 0.0001, L"");//7

			processes_dots.AddGraphParametric(real(point_blood), imag(point_blood), L"Blood flow");
			processes_dots.AddGraphParametric(real(point_calc), imag(point_calc), L"Oscillating object");
			processes_dots.AddGraphParametric(real(point_cavi), imag(point_cavi), L"Cavitating object");
			processes_dots.AddGraphParametric(real(point_stationary_obj), imag(point_stationary_obj), L"Stationary object");

			processes_dots.AddGraphUniform(real(burst), initial_phase_shift, sample_rate, L"Re burst");
			processes_dots.AddGraphUniform(imag(burst), initial_phase_shift, sample_rate, L"Im burst");
			processes_dots.SetScale(range2_F64(-1, -15, 1, 15));

			signals_Complex_plane.AddGraphParametric(real(vector_blood), imag(vector_blood), L"Blood flow vector");
			signals_Complex_plane.AddGraphParametric(real(vector_calc), imag(vector_calc), L"Blood flow vector");
			signals_Complex_plane.AddGraphParametric(real(vector_cavi), imag(vector_cavi), L"Cavitation vector");
			signals_Complex_plane.AddGraphParametric(real(vector_stationary_obj), imag(vector_stationary_obj), L"Stationary vector");
			signals_Complex_plane.AddGraphParametric(real(path_blood), imag(path_blood), L"Blood flow path");
			signals_Complex_plane.AddGraphParametric(real(path_calc), imag(path_calc), L"Blood flow path");
			signals_Complex_plane.AddGraphParametric(real(path_cavi), imag(path_cavi), L"Cavitation path");
			signals_Complex_plane.AddGraphParametric(real(path_stationary_obj), imag(path_stationary_obj), L"Stationary path");
			signals_Complex_plane.SetScale(range2_F64(-1, -1, 1, 1));

			signals_Cartesian_plane.AddGraphUniform(real(path_blood), 1, 0, L"Re blood");
			signals_Cartesian_plane.AddGraphUniform(real(path_calc), 1, 0, L"Re calc");
			signals_Cartesian_plane.AddGraphUniform(real(path_cavi), 1, 0, L"Re cavi");
			signals_Cartesian_plane.AddGraphUniform(real(path_stationary_obj), 1, 0, L"Re stationary obj");
			signals_Cartesian_plane.AddGraphUniform(imag(path_blood), 1, 0, L"Im blood");
			signals_Cartesian_plane.AddGraphUniform(imag(path_calc), 1, 0, L"Im calc");
			signals_Cartesian_plane.AddGraphUniform(imag(path_cavi), 1, 0, L"Im cavi");
			signals_Cartesian_plane.AddGraphUniform(imag(path_stationary_obj), 1, 0, L"Im stationary obj");
			signals_Cartesian_plane.SetScale(range2_F64(-1, 0, 1, 20 * pi()));
		}
		processes_dots.ChangeGraphParametric(4, real(point_blood), imag(point_blood), L"Blood flow");
		processes_dots.ChangeGraphParametric(5, real(point_calc), imag(point_calc), L"Oscillating object");
		processes_dots.ChangeGraphParametric(6, real(point_cavi), imag(point_cavi), L"Cavitating object");
		processes_dots.ChangeGraphParametric(7, real(point_stationary_obj), imag(point_stationary_obj), L"Stationary object");
		processes_dots.ChangeGraphUniform(8, real(burst), initial_phase_shift, sample_rate, L"Re burst");
		processes_dots.ChangeGraphUniform(9, imag(burst), initial_phase_shift, sample_rate, L"Im burst");

		signals_Complex_plane.ChangeGraphParametric(0, real(vector_blood), imag(vector_blood), L"Blood flow vector");
		signals_Complex_plane.ChangeGraphParametric(1, real(vector_calc), imag(vector_calc), L"Oscillating vector");
		signals_Complex_plane.ChangeGraphParametric(2, real(vector_cavi), imag(vector_cavi), L"Cavitation vector");
		signals_Complex_plane.ChangeGraphParametric(3, real(vector_stationary_obj), imag(vector_stationary_obj), L"Stationary vector");
		signals_Complex_plane.ChangeGraphParametric(4, real(path_blood), imag(path_blood), L"Blood flow path");
		signals_Complex_plane.ChangeGraphParametric(5, real(path_calc), imag(path_calc), L"Blood flow path");
		signals_Complex_plane.ChangeGraphParametric(6, real(path_cavi), imag(path_cavi), L"Cavitation path");
		signals_Complex_plane.ChangeGraphParametric(7, real(path_stationary_obj), imag(path_stationary_obj), L"Stationary path");

		signals_Cartesian_plane.ChangeGraphUniform(0, real(path_blood), 0, 1, L"Re blood");
		signals_Cartesian_plane.ChangeGraphUniform(1, real(path_calc), 0, 1, L"Re calc");
		signals_Cartesian_plane.ChangeGraphUniform(2, real(path_cavi), 0, 1, L"Re cavi");
		signals_Cartesian_plane.ChangeGraphUniform(3, real(path_stationary_obj), 0, 1, L"Re stationary");
		signals_Cartesian_plane.ChangeGraphUniform(4, imag(path_blood), 0, 1, L"Im blood");
		signals_Cartesian_plane.ChangeGraphUniform(5, imag(path_calc), 0, 1, L"Im calc");
		signals_Cartesian_plane.ChangeGraphUniform(6, imag(path_cavi), 0, 1, L"Im cavi");
		signals_Cartesian_plane.ChangeGraphUniform(7, imag(path_stationary_obj), 0, 1, L"Im stationary");

		size_t	n_steps = 32;
		x_blood += two_pi() / n_steps;
		x_calc = pi() / 4 + 0.25*sin(2 * x_blood);
		if(x_blood > x0 + 4 * pi())
		{
			x_blood = x0;
			vpath_blood.clear();
			vpath_calc.clear();
			vpath_cavi.clear();
			vpath_stationary_obj.clear();
		}
		signals_Complex_plane.Display(false);
		signals_Cartesian_plane.Display(false);
		processes_dots.Display();
	}
}

void dummy2()
{
	size_t	answer(0);
	while(answer != 3)
	{
		GraphSet	gs(L"", L"amplitude", L"value");
		ComplexFunctionF32 burst(10);
		double frequency = 1;//GetUnsigned("Freq", 1, 0, 10);
		double initial_phase_shift(0);
		double amplitude(1);
		for(size_t i = 0; i < burst.size(); ++i)
		{
			burst[i].re = amplitude*cos(frequency*i + initial_phase_shift);
		}
		gs.AddGraphUniform(real(burst), 0, 1, L"burst");
		RealVectorF64 coefficients(2);
		gs.AddGraphUniform(coefficients, 0, 1, L"approximation");

		//	DetectLSPolynomUniformGrid(real(burst), approximation);
		size_t min_arg = GetUnsigned("Min", 0, 0, 10);
		size_t max_arg = GetUnsigned("Max", 1, 0, 10);
		DetectLSUniversalUniformGrid(real(burst), cosine_LS_function(min_arg, max_arg), coefficients);
		//DetectLSUniversalUniformGrid(const RealFunctionF64 &samples, const abstract_LS_basis_function&f, RealVectorF64 &coefficients);
		gs.AddGraphUniform(coefficients, 0, 1, L"coefficients");

		RealFunctionF32 approximation(10);
		for(size_t i = 0; i < burst.size(); ++i)
		{
			approximation[i] = cos(coefficients[1] * (i - min_arg) / (max_arg - min_arg));
		}//cos(n*(x-min_arg)/(max_arg-min_arg));}

		gs.AddGraphUniform(approximation, 0, 1, L"approximation");
		gs.Display();
	}
}


void y()
{

	GraphSet	gs(L"", L"amplitude", L"value");
	RealFunctionF32 x(17, 0);
	for(size_t i = 0; i < x.size(); ++i)
	{
		x[i] = sin(i);
	}
	gs.AddGraphUniform(x, 0, 1, L"x");
	RealFunctionF32 y(16, 0);
	for(size_t i = 0; i < y.size(); ++i)
	{
		y[i] = x[i + 1] - x[i];
	}
	gs.AddGraphUniform(y, 0, 1, L"y");
	gs.Display();
}


XRAD_END
