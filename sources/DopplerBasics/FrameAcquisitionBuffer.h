#ifndef __FrameAcquisitionBuffer_h
#define __FrameAcquisitionBuffer_h

//------------------------------------------------------------------
//
//	created:	2021/01/27	10:43
//	filename: 	FrameAcquisitionBuffer.h
//	file path:	Q:\Projects\CommonSources\DopplerBasics
//	author:		kns
//	
//	purpose:	
//
//------------------------------------------------------------------

XRAD_BEGIN

// инструмент для накопления 
class FrameAcquisitionBuffer : public RealFunction2D_F32
{
public:
	PARENT(RealFunction2D_F32);


	FrameAcquisitionBuffer() = default;

	virtual void	realloc(size_t vs, size_t hs);
	virtual void	resize(size_t vs, size_t hs);
	virtual void	reset();



// 	parent &operator= (parent &x){ m_memory.realloc(x.vsize(), x.hsize()); return parent::operator=(x); }
// 	parent &operator= (parent &&x){ m_memory.realloc(x.vsize(), x.hsize()); return parent::operator=(std::move(x)); }

	void	AcquireFrame(double frame_agility_factor, double axial_blur, double lateral_blur);
	size_t	n_frames_acquired() const { return m_n_frames_acquired; }

private:
	// запрет на использование части родительских методов
	using parent::parent;
	using parent::operator=;

protected:
	RealFunction2D_F32	m_memory;	
	size_t	m_n_frames_acquired;


	//!	Метод ничего существенного в данных не меняет. 
	//	Его задача -- добиться того, чтобы перед обработкой используемая память гарантированно была 
	//	не в свопе. Иначе при обработке первого кадра возможна потеря быстродействия.
	//	Инициализация случайными числами, чтобы исключить оптимизацию доступа и обеспечить обращение
	//	ко всем элементам буфера.	
	virtual void	flush();
};

class FrameAcquisitionBufferStddev : public FrameAcquisitionBuffer
{
public:
	PARENT(FrameAcquisitionBuffer);

	virtual void	realloc(size_t vs, size_t hs) override;
	virtual void	resize(size_t vs, size_t hs) override;
	virtual void	reset() override;



// 	parent &operator= (parent &x){ m_stddev.realloc(x.vsize(), x.hsize()); return parent::operator=(x); }
// 	parent &operator= (parent &&x){ m_stddev.realloc(x.vsize(), x.hsize()); return parent::operator=(std::move(x)); }

	void	AcquireFrame(double frame_agility_factor, double axial_blur, double lateral_blur);
	const auto & dispersion() const { return m_dispersion; }

private:
	RealFunction2D_F32	m_dispersion;// Дисперсия сигнала относительно памяти (!квадрат СКО!). Покажет, насколько сильно скачет сигнал в конкретной точке. Вомзожно, пригодится для улучшения маски фазы ЦДК
	virtual void flush() override;
};



XRAD_END

#endif //__FrameAcquisitionBuffer_h
