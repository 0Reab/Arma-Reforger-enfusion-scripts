/* todo
- somehow display a message for missing items.
- store misssing items and their amounts for later use.
- override onclick method -> add missing items when in arsenal not any storage comp.
*/

class MyComponent : ScriptedWidgetComponent
{	
	[Attribute(desc: "Items to check for MOS", params: "et", uiwidget: UIWidgets.ResourcePickerThumbnail)]
	protected ref array<ref ResourceName> item_list;
	
	[Attribute("1", desc: "Items amount respectively")] // 1 is auto default value for item
	protected ref array<int> item_amount;
	
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{		
		if (!ValidConfig())
		{
			Print("item list and amount array missmatch");
			return false;
		}
		
		ChimeraCharacter player = GetCharacter();
		if (!player) return false;
		
		CheckGadgets(player);
		
		int len = item_list.Count();
		
		for (int i=0; i < len; i++) // for item, amount in list; do
		{
			ResourceName itemPrefab = item_list[i];
			int itemAmount = item_amount[i];
			
			BaseContainerList components = GetPrefabComponents(itemPrefab);
			string itemName = GetPrefabName(components);
			
			int missingAmount = PlayerHasItem(player, itemPrefab, itemAmount);
			
			Show(itemName, missingAmount);
		}
				
		return true;		
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		return true; // -> not implemented
	}
	
	// only rifle + pistol -> for now idek
	//------------------------------------------------------------------------------------------------
	protected bool CheckAmmo(IEntity player)
	{
		/* add -> BaseWeaponComponent.GetWeaponType();
		returns:
		enum EWeaponType (non comprehensive list)
		{
			WT_RIFLE,
			WT_GRENADELAUNCHER,
			WT_SNIPERRIFLE,
			WT_ROCKETLAUNCHER,
			WT_MACHINEGUN,
			WT_HANDGUN,
		} */
		
		SCR_CharacterInventoryStorageComponent inventory = SCR_CharacterInventoryStorageComponent.Cast(player.FindComponent(SCR_CharacterInventoryStorageComponent));
		if (!inventory)
			return false;
		
		BaseWeaponComponent weaponComp = BaseWeaponComponent.Cast(inventory.FindComponent(BaseWeaponComponent));
		if (!weaponComp)
			return false;
		
		array<BaseMuzzleComponent> muzzles = {};
		weaponComp.GetMuzzlesList(muzzles);
		Print(muzzles);
		
		foreach (BaseMuzzleComponent muzzleComp : muzzles)
		{
			if (!muzzleComp)
				continue;
			
			if (muzzleComp.IsDisposable())
				continue;
			
			EWeaponType wpnType = weaponComp.GetWeaponType();
			
			switch (wpnType)
			{
				case EWeaponType.WT_RIFLE: // do int 10
				case EWeaponType.WT_SNIPERRIFLE: // do int 5 idk
				case EWeaponType.WT_MACHINEGUN: // do int 4 idk
				case EWeaponType.WT_HANDGUN: // do int 4
			}
			
			int magCount = inventory.GetMagazineCountByWeapon(weaponComp);
			
			// check loaded mag
			// by BaseWeaponComponent.GetWeaponType() check against 10 and 4 -> (AR, pistol)
		}

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Show(string itemName, int missingAmount)
	{		
		if (missingAmount > 0)
			PrintFormat("Add %1 %2x", itemName, missingAmount);
		
		if (missingAmount < 0)
			PrintFormat("Remove %1 %2x", itemName, Math.AbsInt(missingAmount));
	}
	
	// only need ChimeraCharacter for the inventory atm.
	//------------------------------------------------------------------------------------------------
	protected ChimeraCharacter GetCharacter()
	{
		PlayerController player = GetGame().GetPlayerController();
		if (!player) return null;
		
		ChimeraCharacter chimera = ChimeraCharacter.Cast(player.GetControlledEntity());
		if (!chimera) return null;
		
		return chimera;
		
		/*CharacterControllerComponent comp = chimera.GetCharacterController();
		if (!comp) return null;
		SCR_CharacterControllerComponent character = SCR_CharacterControllerComponent.Cast(comp);
		if (!character) return null;*/
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CheckGadgets(IEntity player)
	{
		if (!player) return;
		
		SCR_GadgetManagerComponent gadgetManager = SCR_GadgetManagerComponent.Cast(player.FindComponent(SCR_GadgetManagerComponent));
		if (!gadgetManager) return;

		IEntity binos = gadgetManager.GetGadgetByType(EGadgetType.BINOCULARS); 
		IEntity watch = gadgetManager.GetGadgetByType(EGadgetType.WRISTWATCH);
		IEntity nvgs = gadgetManager.GetGadgetByType(EGadgetType.NIGHT_VISION); 
			
		if (binos && watch && nvgs) return; // you have all needed gadgets - good
		
		Print(binos); Print(watch); Print(nvgs); // debug
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool ValidConfig()
	{
		return item_list.Count() == item_amount.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	protected string GetPrefabName(BaseContainerList components)
	{
		int len = components.Count();
		
		for (int idx = 0; idx < len; idx++)
		{
		    BaseContainer currComp = components.Get(idx);
		    if (currComp.GetClassName() == "InventoryItemComponent")
			{
				BaseContainer attributes = currComp.GetObject("Attributes");
				if (attributes)
				{
					BaseContainer itemDisplayName = attributes.GetObject("ItemDisplayName");
		            if (itemDisplayName)
		            {
		                string itemName;
		                itemDisplayName.Get("Name", itemName);
						string translated = WidgetManager.Translate(itemName);
						
		                return translated;
					}
				}
			}
		}
		return "item name error";
	}
	
	//------------------------------------------------------------------------------------------------
	protected BaseContainerList GetPrefabComponents(ResourceName itemPrefab)
	{
		Resource res = Resource.Load(itemPrefab);
		BaseResourceObject resource = res.GetResource();
		BaseContainer container = resource.ToBaseContainer();
		BaseContainerList components = container.GetObjectArray("components");
		
		return components;
	}
	
	//------------------------------------------------------------------------------------------------
	protected int PlayerHasItem(IEntity player, ResourceName itemPrefab, int amountGoal=1)
	{
	    // Get the player's inventory component
	    InventoryStorageManagerComponent inventory = InventoryStorageManagerComponent.Cast(player.FindComponent(InventoryStorageManagerComponent));
	    if (!inventory) return false; // bool on int method?
		
		// parse inventory slots
	    SCR_PrefabNamePredicate searchPredicate();
	    searchPredicate.prefabName = itemPrefab;    
		
	    int amountCurrent = inventory.CountItem(searchPredicate);
		
		if (hasFrag(player, itemPrefab))
			amountCurrent += 1; // increment for the frag in weapon slot
						
	    return amountGoal - amountCurrent; // item count missing to satisfy amount argument
	}
	
	//------------------------------------------------------------------------------------------------
	protected int hasFrag(IEntity player, ResourceName itemPrefab)
	{
		if (!player || !itemPrefab) return 0;
		
		SCR_CharacterInventoryStorageComponent characterInventory = SCR_CharacterInventoryStorageComponent.Cast(player.FindComponent(SCR_CharacterInventoryStorageComponent));
		if (!characterInventory) return 0;
		
		// parse frag slot
		IEntity entity = characterInventory.GetItemFromQuickSlot(3);
		if (!entity) return 0;
		
		EntityPrefabData prefabData = entity.GetPrefabData();
		if (!prefabData) return 0;
	
		ResourceName prefabName = prefabData.GetPrefabName();			
		if (prefabName == itemPrefab)
			return 1;
		
		return 0;
	}
}
