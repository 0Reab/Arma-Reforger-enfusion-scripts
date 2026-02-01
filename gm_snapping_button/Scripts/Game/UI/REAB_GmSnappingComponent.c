class REAB_GmSnappingComponent : ScriptedWidgetComponent
{
	private static const string GAME_MASTER = "Game Master";
	private static const string PREVIEW_VERTICAL_SNAP = "m_PreviewVerticalSnap";
	
	
	override bool OnClick(Widget w, int x, int y, int button)
	{
		BaseContainer editorSettings = GetGame().GetGameUserSettings().GetModule("SCR_EditorSettings");
		if (!editorSettings) 
			return false;
		
		bool state;
		
		editorSettings.Get(PREVIEW_VERTICAL_SNAP, state);
		state = !state; // toggle
		editorSettings.Set(PREVIEW_VERTICAL_SNAP, state);
		
		SCR_AnalyticsApplication.GetInstance().ChangeSetting(GAME_MASTER, PREVIEW_VERTICAL_SNAP);

		GetGame().UserSettingsChanged();
		
		string name = "Snap entities to terrain:";
		string desc = "";
		
		if (state)
			desc = "On";
		else
			desc = "Off";
		
		float duration = 1;
		bool isSilent = true;

		SCR_HintManagerComponent hintManager = SCR_HintManagerComponent.GetInstance();
		if (hintManager)
			hintManager.ShowCustomHint(desc, name, duration, isSilent);
		
		return true;
	}
}