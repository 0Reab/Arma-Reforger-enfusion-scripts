class REAB_TeleportPlayerComponentClass: ScriptComponentClass {}

class REAB_TeleportPlayerComponent: ScriptComponent
{
	protected IEntity m_TeleportReciever;
	protected vector destination[4];
	protected string m_EntityName = "REAB_TeleportReciever";
	
	[RplProp()]
	bool REAB_passed_check;

	//------------------------------------------------------------------------------------------------
	void update_pass(bool state)
	{
		REAB_passed_check = state;
		Replication.BumpMe();
		TeleportPlayer();
		Rpc(RpcDo_Teleport);
	}
	
	//------------------------------------------------------------------------------------------------
    [RplRpc(RplChannel.Reliable, RplRcver.Owner)]
    void RpcDo_Teleport()
    {
        Print("teleporting ...");
		GetTeleportPosition();
        TeleportPlayer();
		Print(destination);
    }
	
	//------------------------------------------------------------------------------------------------
    protected void GetTeleportPosition()
	{
		m_TeleportReciever = GetGame().FindEntity(m_EntityName);
		
		if (!m_TeleportReciever)
			return;
		
		m_TeleportReciever.GetTransform(destination);
	}
	
	//------------------------------------------------------------------------------------------------
    protected void TeleportPlayer()
	{
		IEntity player = GetOwner();
		if (destination[3] == vector.Zero)
			return;
		
		vector previousOrigin = player.GetOrigin();
	
	    BaseGameEntity baseGameEntity = BaseGameEntity.Cast(player);
	    if (baseGameEntity && !BaseVehicle.Cast(baseGameEntity))
	    {
	        baseGameEntity.Teleport(destination);
	    }
	    else
	    {
	        player.SetWorldTransform(destination);
	    }
		
		RplComponent replication = RplComponent.Cast(player.FindComponent(RplComponent));
	    if (replication)
		{
	        replication.ForceNodeMovement(previousOrigin);
		}
		
	    if (!ChimeraCharacter.Cast(player))
	        player.Update();
		
		Print("finished tp method");
	}
}
