#include "DualSenseManager.h"

void DualSenseManager::InitDevice()
{
	DS5W::DeviceEnumInfo infos[16];
	unsigned int         controllersCount = 0;
	DS5W::enumDevices(infos, 16, &controllersCount);
	if (DS5W_SUCCESS(DS5W::initDeviceContext(&infos[0], &con))) {
		logger::info("DualSense connected");
	} else {
		logger::info("DualSense not connected");
	}
}

bool DualSenseManager::SetState(XINPUT_VIBRATION* pVibration)
{
	DS5W::DS5InputState  inState;
	DS5W::DS5OutputState outState;
	ZeroMemory(&inState, sizeof(DS5W::DS5InputState));
	ZeroMemory(&outState, sizeof(DS5W::DS5OutputState));

	if (DS5W_SUCCESS(DS5W::getDeviceInputState(&con, &inState))) {
		auto inputManager = RE::BSInputDeviceManager::GetSingleton();
		auto UI = RE::UI::GetSingleton(); // potentially not thread safe
		if (inputManager->IsGamepadEnabled() && !UI->GameIsPaused()) {

			float leftVibration = (float)pVibration->wLeftMotorSpeed / 65535;
			float rightVibration = (float)pVibration->wRightMotorSpeed / 65535;

			auto player = RE::PlayerCharacter::GetSingleton();
			auto state = player->actorState1.meleeAttackState;

			bool castingDual;
			player->GetGraphVariableBool("IsCastingDual", castingDual);

			bool castingRight;
			player->GetGraphVariableBool("IsCastingRight", castingRight);

			bool allowRotation;
			player->GetGraphVariableBool("bAllowRotation", allowRotation);

			bool usingCrossbow = false;
			if (auto object = player->GetEquippedObject(0)) {
				if (auto weapon = object->As<RE::TESObjectWEAP>()) {
					usingCrossbow = weapon->IsCrossbow();
				}
			}

			if (state == RE::ATTACK_STATE_ENUM::kBowReleased && usingCrossbow) {
				rightVibration = 0.8f;
				leftVibration = 0.2f;
				outState.rightTriggerEffect.effectType = DS5W::TriggerEffectType::NoResitance;
				outState.leftTriggerEffect.effectType = DS5W::TriggerEffectType::NoResitance;
			} else if (!usingCrossbow && (state == RE::ATTACK_STATE_ENUM::kBowAttached ||
				state == RE::ATTACK_STATE_ENUM::kBowDraw ||
				state == RE::ATTACK_STATE_ENUM::kBowFollowThrough ||
				state == RE::ATTACK_STATE_ENUM::kBowNextAttack ||
				state == RE::ATTACK_STATE_ENUM::kBowReleased ||
				state == RE::ATTACK_STATE_ENUM::kBowReleasing)) {
				outState.rightTriggerEffect.effectType = DS5W::TriggerEffectType::ContinuousResitance;
				outState.rightTriggerEffect.Continuous.startPosition = 64;
				outState.rightTriggerEffect.Continuous.force = static_cast<unsigned char>(pow(rightVibration * 255, 2));
			} else if (state == RE::ATTACK_STATE_ENUM::kBowDrawn) {
				outState.rightTriggerEffect.effectType = DS5W::TriggerEffectType::ContinuousResitance;
				outState.rightTriggerEffect.Continuous.startPosition = 64;
				outState.rightTriggerEffect.Continuous.force = usingCrossbow? 64 : 255;
			} else if (castingRight || castingDual || state == RE::ATTACK_STATE_ENUM::kBash) {
				outState.rightTriggerEffect.effectType = DS5W::TriggerEffectType::EffectEx;
				outState.rightTriggerEffect.EffectEx.frequency = static_cast<unsigned char>(rightVibration * 255);
				outState.rightTriggerEffect.EffectEx.startPosition = 64;
				outState.rightTriggerEffect.EffectEx.beginForce = 255;
				outState.rightTriggerEffect.EffectEx.middleForce = 255;
				outState.rightTriggerEffect.EffectEx.endForce = 255;
				outState.rightTriggerEffect.EffectEx.keepEffect = true;
			} else if (allowRotation && !player->IsSneaking()) {
				outState.rightTriggerEffect.effectType = DS5W::TriggerEffectType::EffectEx;
				outState.rightTriggerEffect.EffectEx.frequency = static_cast<unsigned char>(rightVibration * 255 * 0.25);
				outState.rightTriggerEffect.EffectEx.startPosition = 64;
				outState.rightTriggerEffect.EffectEx.beginForce = 255;
				outState.rightTriggerEffect.EffectEx.middleForce = 255;
				outState.rightTriggerEffect.EffectEx.endForce = 255;
				outState.rightTriggerEffect.EffectEx.keepEffect = true;
			}

			bool castingLeft;
			player->GetGraphVariableBool("IsCastingLeft", castingLeft);

			if (castingLeft || castingDual || state == RE::ATTACK_STATE_ENUM::kBash) {
				outState.leftTriggerEffect.effectType = DS5W::TriggerEffectType::EffectEx;
				outState.leftTriggerEffect.EffectEx.frequency = static_cast<unsigned char>(rightVibration * 255);
				outState.leftTriggerEffect.EffectEx.startPosition = 64;
				outState.leftTriggerEffect.EffectEx.beginForce = 255;
				outState.leftTriggerEffect.EffectEx.middleForce = 255;
				outState.leftTriggerEffect.EffectEx.endForce = 255;
				outState.leftTriggerEffect.EffectEx.keepEffect = true;
			} else if (allowRotation && !player->IsSneaking()) {
				outState.leftTriggerEffect.effectType = DS5W::TriggerEffectType::EffectEx;
				outState.leftTriggerEffect.EffectEx.frequency = static_cast<unsigned char>(rightVibration * 255 * 0.25);
				outState.leftTriggerEffect.EffectEx.startPosition = 64;
				outState.leftTriggerEffect.EffectEx.beginForce = 255;
				outState.leftTriggerEffect.EffectEx.middleForce = 255;
				outState.leftTriggerEffect.EffectEx.endForce = 255;
				outState.leftTriggerEffect.EffectEx.keepEffect = true;
			} 

			rightVibration = rightVibration / 10.0f;
			outState.leftRumble = static_cast<unsigned char>(((leftVibration / 2) + rightVibration) * 255);
			outState.rightRumble = static_cast<unsigned char>((leftVibration + (rightVibration / 2)) * 255);
		} else {
			outState.rightTriggerEffect.effectType = DS5W::TriggerEffectType::NoResitance;
			outState.leftTriggerEffect.effectType = DS5W::TriggerEffectType::NoResitance;
		}

		DS5W::setDeviceOutputState(&con, &outState);
		return true;
	}
	return false;
}

bool DualSenseManager::SetStateMenu()
{
	DS5W::DS5InputState  inState;
	DS5W::DS5OutputState outState;
	ZeroMemory(&inState, sizeof(DS5W::DS5InputState));
	ZeroMemory(&outState, sizeof(DS5W::DS5OutputState));

	if (DS5W_SUCCESS(DS5W::getDeviceInputState(&con, &inState))) {
		outState.rightTriggerEffect.effectType = DS5W::TriggerEffectType::NoResitance;
		outState.leftTriggerEffect.effectType = DS5W::TriggerEffectType::NoResitance;
		DS5W::setDeviceOutputState(&con, &outState);
		return true;
	}
	return false;
}
