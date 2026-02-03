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
		if (destination[3] == vector.Zero)
			return false;
		
		IEntity user = GetOwner();
		
		vector previousOrigin = user.GetOrigin();
	
		ChimeraCharacter character = ChimeraCharacter.Cast(user);
		if (!character || character.IsInVehicle())
			return false;
		
	    BaseGameEntity gameEntity = BaseGameEntity.Cast(user);
	    if (gameEntity)
			gameEntity.Teleport(destination);
		
		RplComponent rpl = RplComponent.Cast(user.FindComponent(RplComponent));
	    if (rpl)
	        rpl.ForceNodeMovement(previousOrigin);
		
	    if (!ChimeraCharacter.Cast(user))
	        user.Update();
		
		destination[3] = vector.Zero;
		return true;
	}
}
