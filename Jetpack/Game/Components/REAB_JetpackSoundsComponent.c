[ComponentEditorProps(category: "REAB_ScriptComponents/Character", description: "Jetpack script component for sounds")]
class REAB_JetpackSoundsComponentClass : ScriptComponentClass {}


class REAB_JetpackSoundsComponent : ScriptComponent
{
	protected AudioHandle m_AudioHandle = AudioHandle.Invalid;
	protected bool isPlaying = false;
	
	// define sound component here later
	// shift thrust is bugd, but movement is ok? check other script method calls
	
	// #### PUBLIC METHODS ####	
	
	//------------------------------------------------------------------------------------------------	
	void PlaySound_REAB()
	{
		if (isPlaying) {return;}
		isPlaying = true;
		
		if (Replication.IsClient())
		{
			Rpc(RpcAsk_PlaySound);
			return;
		}
		
		Rpc(RpcDo_PlaySound);
		PlaySound_Internal();	
	}
	
	//------------------------------------------------------------------------------------------------
	void StopSound_REAB()
	{
		if (!isPlaying) {return;}
		isPlaying = false;
		
		if (Replication.IsClient())
		{
			Rpc(RpcAsk_StopSound);
			return;
		}
		
		Rpc(RpcDo_StopSound);
		StopSound_Internal();
	}
	//------------------------------------------------------------------------------------------------
	
	// #### INTERNAL METHODS ####
	
	//------------------------------------------------------------------------------------------------
	void StopSound_Internal()
	{
		CharacterSoundComponent sound = CharacterSoundComponent.Cast(GetOwner().FindComponent(CharacterSoundComponent));
		if (!sound) { return; }
		
		sound.Terminate(m_AudioHandle);
		m_AudioHandle = AudioHandle.Invalid;
		isPlaying = false;
	}
	//------------------------------------------------------------------------------------------------	
	void PlaySound_Internal()
	{
		CharacterSoundComponent sound = CharacterSoundComponent.Cast(GetOwner().FindComponent(CharacterSoundComponent));
		if (!sound) { return; }
				
		if (m_AudioHandle == AudioHandle.Invalid || !isPlaying)
		{
			m_AudioHandle = sound.SoundEvent("SOUND_JETPACK_THRUST");	
			isPlaying = true;	
		}	
	}
	//------------------------------------------------------------------------------------------------
	
	// #### RPC METHODS ####
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_PlaySound()
	{
		Rpc(RpcDo_PlaySound);
		PlaySound_Internal();
	}
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_PlaySound()
	{
		PlaySound_Internal();
	}
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_StopSound()
	{
		Rpc(RpcDo_StopSound);
		StopSound_Internal();
	}
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_StopSound()
	{
		StopSound_Internal();
	}
	//------------------------------------------------------------------------------------------------
}
