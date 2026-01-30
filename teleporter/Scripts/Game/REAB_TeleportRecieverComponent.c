[ComponentEditorProps(category: "REAB", description: "Reciever entity for teleporter")]
class REAB_TeleportRecieverComponentClass : ScriptComponentClass {}


class REAB_TeleportRecieverComponent : ScriptComponent
{
	static const string teleportName = "REAB_TeleportReciever";
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		IEntity teleport = GetGame().FindEntity(teleportName);
		
		if (teleport)
		{
			SCR_EntityHelper.DeleteEntityAndChildren(teleport);
		}
		
		owner.SetName(teleportName);
	}
}