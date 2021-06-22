#include "pre.h"
#include <XRADBasic/MathFunctionTypesMD.h>
#include "S500_CFMFrameSet.h"
#include "S500_CFMRawDataDisplay.h"
#include <XRADGUI/Sources/Gui/MathFunctionGUIMD.h>

#if RUN_CFM_MODES
#include <DopplerBasics/CFM/WallFilters/WallFiltersInteractive.h>
#endif

XRAD_BEGIN


void	ApplyWallFilter(cfm_container_t& cfm_frames, size_t n_cfm_shots)
{
#if RUN_CFM_MODES

	size_t	n_frames = cfm_frames.sizes(0);
	size_t	n_cfm_beams = cfm_frames.sizes(1) / n_cfm_shots;
	size_t	n_cfm_samples = cfm_frames.sizes(2);

	cfm_container_t	cfm_shot_frames;
	cfm_shot_frames.UseData(&cfm_frames.at({ 0,0,0 }), { n_frames, n_cfm_beams, n_cfm_shots, n_cfm_samples }, 1);

	auto	wf = GetWallFilterInteractive(n_cfm_shots);

	for (size_t frame = 0; frame < n_frames; ++frame)
	{
		for (size_t beam = 0; beam < n_cfm_beams; ++beam)
		{
			for (size_t sample = 0; sample < n_cfm_samples; ++sample)
			{
				auto	burst = cfm_shot_frames.GetRow({ frame, beam, slice_mask(0), sample });
				ComplexFunctionF32	cburst(burst);
				wf->Apply(cburst);
				burst.CopyData(cburst);
			}
		}
	}
#endif
}

void	DisplayCFMDetailed(const cfm_container_t& in_cfm_frames, size_t n_cfm_shots, string title, const ScanConverterOptions& sco_cfm)
{
	auto	cfm_frames(in_cfm_frames);

	if (cfm_frames.sizes(1) % n_cfm_shots)
	{
		Error("Invalid n_cfm_shots");
		return;
	}
#if RUN_CFM_MODES
	if (YesOrNo("Apply wall filter before display?", saved_default_value))
	{
		ApplyWallFilter(cfm_frames, n_cfm_shots);
	}
#endif
	size_t	n_frames = cfm_frames.sizes(0);
	size_t	n_cfm_beams = cfm_frames.sizes(1) / n_cfm_shots;
	size_t	n_cfm_samples = cfm_frames.sizes(2);

	size_t	fixed_frame = n_frames / 2;
	size_t	fixed_ray = n_cfm_beams / 2;
	size_t	fixed_sample = n_cfm_samples / 2;
	size_t	fixed_shot = n_cfm_shots / 2;

	size_t answer(0);
	enum answers
	{
		e_everything,
		e_fixed_frame,
		e_fixed_ray,
		e_fixed_sample,
		e_fixed_shot,
		e_exit
	};

	cfm_container_t::invariable	subset;
	cfm_container_t::invariable	cfm_shot_frames;
	cfm_shot_frames.UseData(&cfm_frames.at({ 0,0,0 }), { n_frames, n_cfm_beams, n_cfm_shots, n_cfm_samples }, 1);


	while (answer != e_exit)
	{
		answer = GetButtonDecision(title, //e_exit+1,
			{
			"Everything",
			"Fixed frame",
			"Fixed ray",
			"Fixed sample",
			"Fixed shot",
			"Exit"
			}
		);

		switch (answer)
		{
		case e_everything:
			DisplayMathFunction3D(cfm_frames, title + " / CFM-frames", sco_cfm);
			break;
		case e_fixed_frame:
		{
			fixed_frame = GetUnsigned("Frame no", long(fixed_frame), 0, long(n_frames - 1));
			cfm_shot_frames.GetSubset(subset, { fixed_frame, slice_mask(2), slice_mask(0), slice_mask(1) });
			DisplayMathFunction3D(subset, title + ssprintf(" / Fixed frame #%d", fixed_frame));
		}
		break;
		case e_fixed_ray:
		{
			fixed_ray = GetUnsigned("ray no", long(fixed_ray), 0, long(n_cfm_beams - 1));
			cfm_shot_frames.GetSubset(subset, { slice_mask(2), fixed_ray, slice_mask(0), slice_mask(1) });
			DisplayMathFunction3D(subset, title + ssprintf(" / Fixed ray #%d", fixed_ray));
		}
		break;
		case e_fixed_sample:
		{
			fixed_sample = GetUnsigned("sample no", long(fixed_sample), 0, long(n_cfm_samples - 1));
			cfm_shot_frames.GetSubset(subset, { slice_mask(1), slice_mask(2), slice_mask(0), fixed_sample });
			DisplayMathFunction3D(subset, title + ssprintf(" / Fixed sample #%d", fixed_sample));
		}
		break;
		case e_fixed_shot:
		{
			fixed_shot = GetUnsigned("shot no", long(fixed_shot), 0, long(n_cfm_shots - 1));
			cfm_shot_frames.GetSubset(subset, { slice_mask(0), slice_mask(1), fixed_shot, slice_mask(2) });
			DisplayMathFunction3D(subset, title + ssprintf(" / Fixed shot #%d", fixed_shot), sco_cfm);
		}
		break;
		};
	}
}



void	DisplayCFMFrameSet(const S500_CFMFrameSet& frames, wstring title)
{
	size_t	option = 0;
	ScanConverterOptions sco_cfm;

	enum { cfm_frames, b_frames, exit, n_options };
	do
	{
		option = GetButtonDecision(wstring_to_string(title, e_encode_literals), /*n_options,*/{ "CFM-frames", "B-frames", "Exit" });
		try
		{
			switch (option)
			{
			case cfm_frames:
				DisplayCFMDetailed(frames.cfm_frames, frames.n_cfm_shots(), "CFM-frames", frames.sco_cfm);
				break;
			case b_frames:
				DisplayMathFunction3D(frames.b_frames, "B-frames", frames.sco_b);
				break;
			case exit:
				break;
			}
		}
		catch (canceled_operation&) {}
	} while (option != n_options - 1);
}

XRAD_END


