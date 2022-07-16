#pragma once

#include <API/ds5w.h>
#include <xinput.h>

class DualSenseManager
{
public:
	static void InstallHooks()
	{
		Hooks::Install();
	}

	static DualSenseManager* GetSingleton()
	{
		static DualSenseManager handler;
		return &handler;
	}

	DS5W::DeviceContext con;

	void InitDevice();
	bool SetState(XINPUT_VIBRATION* pVibration);
	bool SetStateMenu();

protected:
	struct Hooks
	{
		struct XInput_SetState_Game
		{
			static DWORD thunk(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
			{
				if (GetSingleton()->SetState(pVibration))
					return NULL;
				return func(dwUserIndex, pVibration);
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		struct XInput_SetState_Menu
		{
			static DWORD thunk(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
			{
				GetSingleton()->SetStateMenu();
				return func(dwUserIndex, pVibration);
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		static void Install()
		{
			stl::write_thunk_call<XInput_SetState_Game>(REL::RelocationID(67223, 68532).address() + REL::Relocate(0x25C, 0x313));
			stl::write_thunk_call<XInput_SetState_Menu>(REL::RelocationID(67224, 68533).address() + REL::Relocate(0x11, 0x11));
		}
	};

private:
	constexpr DualSenseManager() noexcept = default;
	DualSenseManager(const DualSenseManager&) = delete;
	DualSenseManager(DualSenseManager&&) = delete;

	~DualSenseManager() = default;

	DualSenseManager& operator=(const DualSenseManager&) = delete;
	DualSenseManager& operator=(DualSenseManager&&) = delete;
};
