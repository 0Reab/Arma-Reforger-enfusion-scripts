[ComponentEditorProps(category: "REAB", description: "Spawns shockwave effect on init and deletes itself")]
class REAB_ShockwaveComponentClass : ScriptComponentClass {}


class REAB_ShockwaveComponent : ScriptComponent
{
	protected float time;
	
	[Attribute("0.5", desc: "Shockwave duration limit in seconds.")]
	protected float limit;
	
	[Attribute("5", desc: "Shockwave expansion speed modifier each frame: (current scale + this_value).")]
	protected float factor;
	
	
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.FRAME);
		Print(owner.GetScale());
	}
	
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		time += timeSlice;

		owner.SetScale(owner.GetScale() + factor);
		
		if (time > limit)
			SCR_EntityHelper.DeleteEntityAndChildren(owner);
	}
}