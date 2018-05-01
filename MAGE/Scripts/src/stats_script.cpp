//-----------------------------------------------------------------------------
// Engine Includes
//-----------------------------------------------------------------------------
#pragma region

#include "stats_script.hpp"
#include "system\system_usage.hpp"
#include "exception\exception.hpp"

#pragma endregion

//-----------------------------------------------------------------------------
// Engine Definitions
//-----------------------------------------------------------------------------
namespace mage::script {

	StatsScript::StatsScript()
		: BehaviorScript(),
		m_text(), 
		m_accumulated_nb_frames(0u), 
		m_prev_wall_clock_time(0.0), 
		m_prev_core_clock_time(0.0), 
		m_fps(0u), 
		m_spf(0.0f), 
		m_cpu(0.0f), 
		m_ram(0u) {}

	StatsScript::StatsScript(const StatsScript& script) noexcept = default;

	StatsScript::StatsScript(StatsScript&& script) noexcept = default;
	
	StatsScript::~StatsScript() = default;

	StatsScript& StatsScript
		::operator=(const StatsScript& script) noexcept = default;

	StatsScript& StatsScript
		::operator=(StatsScript&& script) noexcept = default;

	void StatsScript::Load([[maybe_unused]] Engine& engine) {
		ThrowIfFailed(HasOwner(), 
					  "This script needs to be attached to a node.");
		
		m_text = GetOwner()->Get< rendering::SpriteText >();
		ThrowIfFailed((nullptr != m_text), 
					  "This script needs a sprite text component.");
	}

	void StatsScript::Update([[maybe_unused]] Engine& engine) {
		static constexpr F64 s_resource_fetch_period = 1.0;
		
		++m_accumulated_nb_frames;
		const auto wall_clock_time  = engine.GetTime().GetWallClockTotalDeltaTime();
		const auto wall_clock_delta = wall_clock_time - m_prev_wall_clock_time;

		if (s_resource_fetch_period <= wall_clock_delta) {
			const auto core_clock_time  = engine.GetTime().GetCoreClockTotalDeltaTime();
			const auto core_clock_delta = core_clock_time - m_prev_core_clock_time;

			m_fps = static_cast< U32 >(m_accumulated_nb_frames / wall_clock_delta);
			m_spf = static_cast< F32 >(wall_clock_delta / m_accumulated_nb_frames) * 1000.0f;
			m_cpu = static_cast< F32 >(core_clock_delta / wall_clock_delta) * 100.0f;
			m_ram = static_cast< U32 >(GetVirtualMemoryUsage() >> 20u);

			m_accumulated_nb_frames = 0u;
			m_prev_wall_clock_time  = wall_clock_time;
			m_prev_core_clock_time  = core_clock_time;
		}

		RGBA color = (m_fps > 120u) ? color::Green : color::Red;

		m_text->SetText(wstring(L"FPS: "));
		m_text->AppendText(rendering::ColorString(
			std::to_wstring(m_fps),
			std::move(color)));
		
		wchar_t buffer[64];
		_snwprintf_s(buffer, std::size(buffer), 
			         L"\nSPF: %.2fms\nCPU: %.1f%%\nRAM: %uMB\nDCs: %u", 
					 m_spf, m_cpu, m_ram, rendering::Pipeline::s_nb_draws);
		m_text->AppendText(wstring(buffer));
	}
}