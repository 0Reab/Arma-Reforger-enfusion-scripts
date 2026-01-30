class REAB_TeleportPlayerComponentClass: ScriptComponentClass {}

class REAB_TeleportPlayerComponent: ScriptComponent
{
	protected IEntity m_TeleportReciever;
	protected vector destination[4];
	protected string m_EntityName = "REAB_TeleportReciever";
	

	//------------------------------------------------------------------------------------------------
	bool RpcRequest_Teleport()
	{
		bool pos_result = GetTeleportPosition();
		bool tp_result = TeleportPlayer();
		
		bool success = pos_result && tp_result;
		
		Rpc(RpcDo_Teleport);
		return success;
	}

	//------------------------------------------------------------------------------------------------
    [RplRpc(RplChannel.Reliable, RplRcver.Owner)]
    void RpcDo_Teleport()
    {
		GetTeleportPosition();
        TeleportPlayer();
		//Print(destination);
    }
	
	//------------------------------------------------------------------------------------------------
    protected bool GetTeleportPosition()
	{
		m_TeleportReciever = GetGame().FindEntity(m_EntityName);
		
		if (!m_TeleportReciever)
			return false;
		
		m_TeleportReciever.GetTransform(destination);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
    protected bool TeleportPlayer()
	{
		IEntity player = GetOwner();
		if (destination[3] == vector.Zero)
			return false;
		
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
		
		destination[3] = vector.Zero;
		return true;
	}
}