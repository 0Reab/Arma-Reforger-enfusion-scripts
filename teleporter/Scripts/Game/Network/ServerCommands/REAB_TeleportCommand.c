class REAB_TeleportCommand : ScrServerCommand
{
	protected ref ScrServerCmdResult m_result = new ScrServerCmdResult(string.Empty, EServerCmdResultType.OK);
	protected string m_sPasswordFilePath = "$profile:teleport_password.txt";


	//------------------------------------------------------------------------------------------------
	override event protected ref ScrServerCmdResult OnChatClientExecution(array<string> argv, int playerId)
	{
		return m_result;
	}

	//------------------------------------------------------------------------------------------------
	protected override ref ScrServerCmdResult OnChatServerExecution(array<string> argv, int playerId)
	{
		if (argv.Count() < 2)
		{
			Result(false, "Usage: #tp <password>", true);
			return m_result;
		}

		string inputPassword = argv[1];

		string correctPassword = ReadPasswordFromFile();

		if (correctPassword.IsEmpty())
		{
			Result(false, "Error: Password file not found or empty");
			return m_result;
		}

		if (inputPassword != correctPassword)
		{
			Result(false, "Incorrect password");
			return m_result;
		}

		bool teleportSuccess = ExecuteTeleport(playerId);

		if (teleportSuccess)
			Result(true, "Teleported successfully");
		else
			Result(false, "Teleport failed");

		return m_result;
	}

	//------------------------------------------------------------------------------------------------
	protected void Result(bool ok, string response, bool usage=false)
	{
		if (usage)
			m_result.m_eResultType = EServerCmdResultType.PARAMETERS;
		if (ok)
			m_result.m_eResultType = EServerCmdResultType.OK;
		if (!ok)
			m_result.m_eResultType = EServerCmdResultType.ERR;

		m_result.m_sResponse = response;
	}
	//------------------------------------------------------------------------------------------------
	protected string ReadPasswordFromFile()
	{
		FileHandle file = FileIO.OpenFile(m_sPasswordFilePath, FileMode.READ);
		if (!file)
			return string.Empty;

		string password;
		file.ReadLine(password);
		password.Trim();

		file.Close();
		return password;
	}

	//------------------------------------------------------------------------------------------------
	protected bool ExecuteTeleport(int playerId)
	{
		IEntity playerEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!playerEntity)
			return false;

		REAB_TeleportPlayerComponent teleportComp = REAB_TeleportPlayerComponent.Cast(
			playerEntity.FindComponent(REAB_TeleportPlayerComponent)
		);
		if (!teleportComp)
			return false;

		return teleportComp.RpcRequest_Teleport();
	}
	
	//------------------------------------------------------------------------------------------------
	override ref ScrServerCmdResult OnUpdate()
	{ return ScrServerCmdResult(string.Empty, EServerCmdResultType.OK); }
	
	//------------------------------------------------------------------------------------------------
	override event string GetKeyword()
	{ return "tp"; }
	
	//------------------------------------------------------------------------------------------------
	override event protected int RequiredChatPermission()
	{ return 0; }
	
	//------------------------------------------------------------------------------------------------
	override event bool IsServerSide()
	{ return true; }
}
