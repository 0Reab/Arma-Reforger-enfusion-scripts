class REAB_GmOrientComponent : ScriptedWidgetComponent
{
	private static const string GAME_MASTER = "Game Master";
	private static const string PREVIEW_VERTICLE_MODE = "m_PreviewVerticleMode";
	
	
	override bool OnClick(Widget w, int x, int y, int button)
	{
		BaseContainer editorSettings = GetGame().GetGameUserSettings().GetModule("SCR_EditorSettings");
		if (!editorSettings) 
			return false;
		
		int state;
		
		editorSettings.Get(PREVIEW_VERTICLE_MODE, state);
		
		if (state >= 3) // cycle thru flags 1, 2, 4
			state = 1;
		else
			state = 1 << state;
		
		editorSettings.Set(PREVIEW_VERTICLE_MODE, state);
		SCR_AnalyticsApplication.GetInstance().ChangeSetting(GAME_MASTER, PREVIEW_VERTICLE_MODE);
		
		GetGame().UserSettingsChanged();
		
		string name = "Orient entities to:";
		string desc = "";
		
		float duration = 1;
		bool isSilent = true;
		
		switch (state)
		{
			case 1: desc = "Sea Level"; break;
			case 2: desc = "Terrain Level"; break;
			case 4: desc = "Geometry"; break;
		}
		
		SCR_HintManagerComponent hintManager = SCR_HintManagerComponent.GetInstance();
		if (hintManager)
			hintManager.ShowCustomHint(desc, name, duration, isSilent);
		
		return true;
	}
}