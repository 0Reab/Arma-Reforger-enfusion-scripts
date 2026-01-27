/* todo
- somehow display a message for missing items.
- store misssing items and their amounts for later use.
- override onclick method -> add missing items when in arsenal not any storage comp.

- fix ammo check method
- frag check method has a bug where if two frags are in inventory but not in weapon slots it will misscoun them as one excess
*/

class MyComponent : ScriptedWidgetComponent
{	
	[Attribute(desc: "Items to check for MOS", params: "et", uiwidget: UIWidgets.ResourcePickerThumbnail)]
	protected ref array<ref ResourceName> item_list;
	
	[Attribute("1", desc: "Items amount respectively")] // 1 is auto default value for item
	protected ref array<int> item_amount;
	
	protected string formattedResult;
	protected ref map<ResourceName, int> resultMap = new map<ResourceName, int>; // name : amount needed (amount can be positive or negative int)
	
	
	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (!ValidConfig())
		{
			Print("item list and amount array missmatch");
			return false;
		}
		
		ChimeraCharacter player = GetCharacter();
		if (!player) return false;
		
		CheckGadgets(player);
		resultMap.Clear();
		
		int len = item_list.Count();

		for (int i=0; i < len; i++) // for item, amount in list; do
		{
			ResourceName itemPrefab = item_list[i];
			int itemAmount = item_amount[i];
			
			BaseContainerList components = GetPrefabComponents(itemPrefab);
			string itemName = GetPrefabName(components);
			
			int missingAmount = PlayerHasItem(player, itemPrefab, itemAmount);
			
			UpdateResult(itemPrefab, itemName, missingAmount);
		}
		
		CheckAmmo(player);
		ShowResult();
		formattedResult = "";
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{				
		return true;		
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AddItems(IEntity player) // only a reminder -> server-side implementation for later
	{
		SCR_InventoryStorageManagerComponent mgr = SCR_InventoryStorageManagerComponent.Cast(player.FindComponent(SCR_InventoryStorageManagerComponent));
		BaseInventoryStorageComponent storage = BaseInventoryStorageComponent.Cast(player.FindComponent(BaseInventoryStorageComponent));

        //mgr.TrySpawnPrefabToStorage(prefab, storage);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateResult(ResourceName itemPrefab, string itemName, int missingAmount)
	{
		if (missingAmount == 0)
			return;
		
		if (missingAmount > 0)
			formattedResult += string.Format("Add %1 %2x\n", itemName, missingAmount);
		
		if (missingAmount < 0)
			formattedResult += string.Format("Remove %1 %2x\n", itemName, Math.AbsInt(missingAmount));
		
		resultMap.Insert(itemPrefab, missingAmount);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ShowResult()
	{
		bool silent = true;
		int duration = 60;
		string name = "Inventory check result";

		SCR_HintManagerComponent hintManager = SCR_HintManagerComponent.GetInstance();
		if (hintManager)
			hintManager.ShowCustomHint(formattedResult, name, duration, silent);
	}
	
	// only rifle + pistol -> for now idek
	//------------------------------------------------------------------------------------------------
	protected bool CheckAmmo(ChimeraCharacter player)
	{	
		int totalMags = 0;
		
		SCR_InventoryStorageManagerComponent inventoryManager = GetInventoryManager(player);
	    if (!inventoryManager)
	        return 0;
		
		BaseWeaponManagerComponent weaponManager = BaseWeaponManagerComponent.Cast(
        	player.FindComponent(BaseWeaponManagerComponent)
		);
		if (!weaponManager)
			return 0;
		
		array<BaseWeaponComponent> weapons = new array<BaseWeaponComponent>;
		weaponManager.GetWeapons(weapons);
		
		int count = 0;
		foreach (BaseWeaponComponent wpn : weapons)
		{
			count ++;
			if (!wpn)
				continue;

			totalMags = inventoryManager.GetMagazineCountByWeapon(wpn);
			
			string weaponType;
			switch (count)
			{
				case 3: weaponType = "Primary"; break;
				case 4: weaponType = "Secondary"; break;
				case 5: weaponType = "Sidearm"; break;
			}
			
			if (IsWpnLoaded(wpn))
				totalMags++; // add one if mag loaded
			
			if (weaponType != "")
				formattedResult += string.Format("%1 ammo %2x\n", weaponType, totalMags);
		}
	    return 0;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsWpnLoaded(BaseWeaponComponent wpn)
	{
		array<BaseMuzzleComponent> muzzles = {};
		wpn.GetMuzzlesList(muzzles);
		    
		foreach (BaseMuzzleComponent muzzle : muzzles)
		{
			if (muzzle && !muzzle.IsDisposable() && muzzle.GetMagazine())
		    {
				return true;
			}
		}
		return false;
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
		
		if (!binos) formattedResult += "Add Binoculars\n"; 
		if (!watch) formattedResult += "Add Watch\n"; 
		if (!nvgs) formattedResult += "Add Night Vision\n"; 
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
	    SCR_InventoryStorageManagerComponent inventory = GetInventoryManager(player);
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
	
	protected SCR_InventoryStorageManagerComponent GetInventoryManager(IEntity player)
	{
		SCR_InventoryStorageManagerComponent inventory = SCR_InventoryStorageManagerComponent.Cast(player.FindComponent(SCR_InventoryStorageManagerComponent));
		return inventory;
	}
}
