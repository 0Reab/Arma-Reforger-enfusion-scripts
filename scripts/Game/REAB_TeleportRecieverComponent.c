[ComponentEditorProps(category: "REAB", description: "Reciever entity for teleporter")]
class REAB_TeleportRecieverComponentClass : ScriptComponentClass {}


class REAB_TeleportRecieverComponent : ScriptComponent
{
	override void OnPostInit(IEntity owner)
	{
		owner.SetName("REAB_TeleportReciever");
	}
}