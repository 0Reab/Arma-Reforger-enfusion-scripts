class MyComponent : ScriptedWidgetComponent
{	
	[Attribute(desc: "Items to check for MOS", params: "et", uiwidget: UIWidgets.ResourcePickerThumbnail)]
	protected ref array<ref ResourceName> item_list;
	
	[Attribute(desc: "Items amount respectively")]
	protected ref array<ref int> item_amount;
	
		
	override bool OnMouseEnter(Widget w, int x, int y)
	{		
		PlayerController player = GetGame().GetPlayerController();
		//Print(player);
				
		ChimeraCharacter chimera = ChimeraCharacter.Cast(player.GetControlledEntity());
		//Print(chimera);
		
		CharacterControllerComponent comp = chimera.GetCharacterController();
		//Print(comp);
		
		SCR_CharacterControllerComponent character = SCR_CharacterControllerComponent.Cast(comp);
		//Print(character);
		
		int len = item_list.Count();
		
		if (len != item_amount.Count())
		{
			Print("item list and amount array missmatch");
			return false;
		}
		
		for (int i=0; i < len; i++)
		{
			ResourceName itemPrefab = item_list[i];
			int itemAmount = item_amount[i];
			
			
			Resource res = Resource.Load(itemPrefab);
			BaseResourceObject resource = res.GetResource();
			BaseContainer container = resource.ToBaseContainer();
			BaseContainerList components = container.GetObjectArray("components");
			
			
			for (int j = 0; j < components.Count(); j++)
			{
			    BaseContainer currComp = components.Get(j);
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
							
			                Print(translated);
						}
					}
				}
			}
		}
		
		return true;		
	}
	
	protected bool PlayerHasItem(IEntity player, ResourceName itemPrefab, int amount=1)
	{
	    // Get the player's inventory component
	    InventoryStorageManagerComponent inventory = InventoryStorageManagerComponent.Cast(player.FindComponent(InventoryStorageManagerComponent));
	    if (!inventory) return false;
		
	    SCR_PrefabNamePredicate searchPredicate();
	    searchPredicate.prefabName = itemPrefab;    
	    int count = inventory.CountItem(searchPredicate);
		
		Print(count);
	    return count > 0;
	}
}