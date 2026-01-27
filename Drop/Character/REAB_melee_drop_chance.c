modded class SCR_CharacterControllerComponent : CharacterControllerComponent
{
	bool DEBUG = false; // enables print statements for debugging
	
	int GetChance()
	{
		RandomGenerator gen = new RandomGenerator();
		int chance = gen.RandIntInclusive(1, 10);
		
		if (DEBUG) PrintFormat("chance -> %1", chance);
		
		return chance;
	}
	
	//--------------------------------------------------------------------------------------------------------------------------------------
	
	WeaponSlotComponent GetWpnSlot()
	{
		BaseWeaponManagerComponent WpnManager = null;
        ChimeraCharacter character = GetCharacter();
		
		WpnManager = BaseWeaponManagerComponent.Cast(character.FindComponent(BaseWeaponManagerComponent));
		WeaponSlotComponent slot = WpnManager.GetCurrentSlot();
		
		if (!character || !WpnManager || !slot)
		{
			if (DEBUG) Print("WpnManager || character || slot -> failed to initialize");
			return null;
		}
		
		return slot;
	}
	
	//--------------------------------------------------------------------------------------------------------------------------------------	
	
	override void OnMeleeDamage(bool started)
	{
		m_MeleeComponent.SetMeleeAttackStarted(started);
		int chance = GetChance();
		
		if (chance == 1)
		{
			WeaponSlotComponent slot = GetWpnSlot();
			
           	DropWeapon(slot);
            if (DEBUG) Print("Weapon dropped!!!");
		}

		//if (GetCanFireWeapon())
	}
}
