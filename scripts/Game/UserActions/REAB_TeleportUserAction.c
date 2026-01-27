// teleport whitelisted player to location of the teleport_station.

class REAB_TeleportUserAction : ScriptedUserAction
{	
	protected string m_File = "$profile:/PlayerWhitelist.json";	
	protected ref array<string> whitelist;
	
	[RplProp()]
	protected bool passed;
	
	//------------------------------------------------------------------------------------------------
	protected UUID GetPlayerID(IEntity player)
	{
		UUID player_id = SCR_PlayerIdentityUtils.GetPlayerIdentityId(player); // player instigator bohemia ID
		PrintFormat("Get UUID = %1", player_id, LogLevel.NORMAL);
		
		return player_id;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void jsonParse(string raw)
	{
		// god forgive me for my sin, no eyes shall be laid up on this creation.
		
		int dictEnd = raw.IndexOf("}");
		string str = raw.Substring(0, dictEnd + 1);
		
		array<string> charlist = {"\n", "{", "}", "\"", " "};
		
		foreach (string char : charlist)
		{
			str.Replace(char, "");	
		}
			
		array<string> list = new array<string>;
		str.Split(",", list, true);
			
		foreach (string pair : list)
		{
			array<string> sublist = new array<string>;
			pair.Split(":", sublist, true);
				
			string key = sublist[1];
				
			whitelist.Insert(key);
			Print(key);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool jsonInstanciate()
	{
		JsonApiStruct json();
		whitelist = new array<string>;
		
		bool success = json.LoadFromFile(m_File);
		
		if (success && json.HasData())
		{
			Print("loaded OK");
			
			string raw = json.AsString();
						
			jsonParse(raw);
			Print(whitelist);	
			return true;
		}
		
		Print("json fail");
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected bool checkPlayer(IEntity pUserEntity)
	{
		// DEBUG VALUE EARLY RETURN
		return true;
		
		UUID playerID = GetPlayerID(pUserEntity);
		
		if (jsonInstanciate())
		{
			if (whitelist.Contains(playerID))
			{
				return true;
			}
		}
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void PerformContinuousAction(IEntity pOwnerEntity, IEntity pUserEntity, float timeSlice)
	{
		REAB_TeleportPlayerComponent teleportComp = REAB_TeleportPlayerComponent.Cast(
			pUserEntity.FindComponent(REAB_TeleportPlayerComponent)
		);
		
		if (!teleportComp)
			return;
		
		if (Replication.IsServer())
		{	
			if (passed == true)
				return;
			
			if (!checkPlayer(pUserEntity))
				return;
			
			passed = true;
			teleportComp.update_pass(passed);
			
			Print(pUserEntity);
		}

		if (!passed)
			return;	
		
		teleportComp.RpcDo_Teleport();
		
		passed = false;
		teleportComp.update_pass(passed);	
	}
}