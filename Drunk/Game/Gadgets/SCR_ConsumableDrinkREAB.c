//! Morphine effect
[BaseContainerProps()]
class SCR_ConsumableDrinkREAB : SCR_ConsumableEffectHealthItems
{
	override void AddConsumableDamageEffects(notnull ChimeraCharacter char, IEntity instigator)
	{
		REAB_DrunkComponent drunkComp = REAB_DrunkComponent.Cast(char.FindComponent(REAB_DrunkComponent));
		if (!drunkComp)
		{
			Print("drunk comp init fail");
			return;
		}
		
		drunkComp.IsDrunk_REAB();
		Print("consumed drink");
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanApplyEffect(notnull IEntity target, notnull IEntity user, out SCR_EConsumableFailReason failReason)
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanApplyEffectToHZ(notnull IEntity target, notnull IEntity user, ECharacterHitZoneGroup group, out SCR_EConsumableFailReason failReason = SCR_EConsumableFailReason.NONE)
	{
		return CanApplyEffect(target, user, failReason);
	}
	
	//------------------------------------------------------------------------------------------------
	EDamageType GetDefaultDamageType()
	{
		return EDamageType.HEALING;
	}
	
	override ItemUseParameters GetAnimationParameters(IEntity item, notnull IEntity target, ECharacterHitZoneGroup group = ECharacterHitZoneGroup.VIRTUAL)
	{
		ItemUseParameters itemUseParams = super.GetAnimationParameters(item, target, group);
		itemUseParams.SetAllowMovementDuringAction(true);
		return itemUseParams;
	}
		
	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_ConsumableDrinkREAB()
	{
		m_eConsumableType = SCR_EConsumableType.MORPHINE;
	}
}
