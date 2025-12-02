[ComponentEditorProps(category: "Movement/Character", description: "script component for enabling/disabling jetpack script")]
class REAB_JetpackDetectComponentClass : ScriptComponentClass {}


class REAB_JetpackDetectComponent : ScriptComponent
{
	protected SCR_CharacterControllerComponent character;
	protected REAB_JetpackFlyingComponent jetpack;
	
	
	override void OnPostInit(IEntity owner)
	{
		Print("test comp OK");
		SetEventMask(owner, EntityEvent.SIMULATE);
	}

	//---------------------------------------------------------------------------------------------------------------
	override void OnAddedToParent(IEntity child, IEntity parent)
	{
		super.OnAddedToParent(child, parent);
		
		Print("added to parent");
		
		if (!parent)
			return;

		jetpack = REAB_JetpackFlyingComponent.Cast(parent.FindComponent(REAB_JetpackFlyingComponent));
		if (!jetpack)
		{
			Print("no jetpack comp");
			return;
		}
			
		character = SCR_CharacterControllerComponent.Cast(parent.FindComponent(SCR_CharacterControllerComponent));
		if (!character)
		{
			Print("no char contrl comp");
			return;
		}
			
		Print(jetpack.Enable_REAB());
		jetpack.PlayParticles_REAB("smoke");
	}
	
	//---------------------------------------------------------------------------------------------------------------
	override void OnRemovedFromParent(IEntity child, IEntity parent)
	{
		super.OnRemovedFromParent(child, parent);
		
		Print("removed from parent");
		
		if (jetpack)
			Print(jetpack.Disable_REAB());
			jetpack.StopParticles_REAB("smoke");
	}
}