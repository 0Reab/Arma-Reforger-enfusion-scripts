[ComponentEditorProps(category: "REAB_ScriptComponents/Jetpack", description: "script component for enabling/disabling jetpack script")]
class REAB_JetpackDetectComponentClass : ScriptComponentClass {}


class REAB_JetpackDetectComponent : ScriptComponent
{
	protected SCR_CharacterControllerComponent character;
	protected REAB_JetpackParticlesComponent particles;
	protected REAB_JetpackFlyingComponent jetpack;
	protected REAB_JetpackSoundsComponent sounds;
	
	
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.SIMULATE);
	}

	//---------------------------------------------------------------------------------------------------------------
	override void OnAddedToParent(IEntity child, IEntity parent)
	{
		super.OnAddedToParent(child, parent);
				
		if (!parent)
			return;

		jetpack = REAB_JetpackFlyingComponent.Cast(parent.FindComponent(REAB_JetpackFlyingComponent));
		if (!jetpack)
			return;
			
		character = SCR_CharacterControllerComponent.Cast(parent.FindComponent(SCR_CharacterControllerComponent));
		if (!character)
			return;
		
		/*particles = REAB_JetpackParticlesComponent.Cast(parent.FindComponent(REAB_JetpackParticlesComponent));
		if (!particles)
			return; */
			
		jetpack.Enable_REAB();
	}
	
	//---------------------------------------------------------------------------------------------------------------
	override void OnRemovedFromParent(IEntity child, IEntity parent)
	{
		super.OnRemovedFromParent(child, parent);
		
		if (jetpack)
			jetpack.Disable_REAB();
	}
}
