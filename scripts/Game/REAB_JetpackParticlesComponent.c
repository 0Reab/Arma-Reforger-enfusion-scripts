[ComponentEditorProps(category: "REAB_ScriptComponents/BaseCharacter", description: "Jetpack script component for particles")]
class REAB_JetpackParticlesComponentClass : ScriptComponentClass {}


class REAB_JetpackParticlesComponent : ScriptComponent
{
	// hashmaps for tracking active particles
	protected ref map<string, ParticleEffectEntity> activeParticles = new map<string, ParticleEffectEntity>();
	protected static ref map<string, ResourceName> PARTICLE_PATHS = new map<string, ResourceName>();
	
	// particles
	//protected static ResourceName SMOKE = "{3D48DA325D288CCF}Particles/Vehicle/Custom_Jetpack_Vehicle_smoke_car_exhaust_damage.ptc";
	protected static ResourceName SMOKE_TRAIL = "{46E002F3F85DB503}Particles/Weapon/Custom_Jetpack_Flare_Parachute.ptc";
	protected static ResourceName FLAME_LEFT = "{DFCB669950C3D7F5}Particles/Weapon/Custom_Jetpack_Trail_Left_M229.ptc";
	protected static ResourceName FLAME_RIGHT = "{A8D906F1965C5277}Particles/Weapon/Custom_Jetpack_Trail_Right_M229.ptc";
	

	// #### PUBLIC METHODS ####
	
	//! \param[in] particleType -> possible string: smoke, smoke_Trail, flame_Left, flame_Right
	//------------------------------------------------------------------------------------------------
	void PlayParticles_REAB(string particleType)
	{
		if (Replication.IsClient())
		{
			Rpc(RpcAsk_PlayParticles, particleType);
			return;
		}
		
		Rpc(RpcDo_PlayParticles, particleType);
		PlayParticles_Internal(particleType);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] particleType -> possible string: smoke, smoke_Trail, flame_Left, flame_Right
	void StopParticles_REAB(string particleType)
	{
		if (Replication.IsClient())
		{
			Rpc(RpcAsk_StopParticles, particleType);
			return;
		}
		
		Rpc(RpcDo_StopParticles, particleType);
		StopParticles_Internal(particleType);
	}	
	
	// #### PUBLIC WRAPPER METHODS ####
	
	//------------------------------------------------------------------------------------------------
	void StopAllParticles_REAB()
	{
		StopParticles_REAB("flame_Left");
		StopParticles_REAB("flame_Right");
		StopParticles_REAB("smoke_Trail");
		StopParticles_REAB("smoke");
	}
	
	//------------------------------------------------------------------------------------------------
	void StopThrusterParticles_REAB(bool stopSmokeTrail)
	{
		StopParticles_REAB("flame_Left");
		StopParticles_REAB("flame_Right");
		
		if (stopSmokeTrail) { StopParticles_REAB("smoke_Trail"); }
	}
	
	//------------------------------------------------------------------------------------------------
	void PlayThrusterParticles_REAB(bool playSmokeTrail)
	{
		PlayParticles_REAB("flame_Left");
		PlayParticles_REAB("flame_Right");
		
		if (playSmokeTrail) { PlayParticles_REAB("smoke_Trail"); }
	}

	// #### RPC METHODS ####
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Unreliable, RplRcver.Server)]
	protected void RpcAsk_PlayParticles(string particleType)
	{
		Rpc(RpcDo_PlayParticles, particleType);
		PlayParticles_Internal(particleType);
	}
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Unreliable, RplRcver.Broadcast)]
	protected void RpcDo_PlayParticles(string particleType)
	{
		PlayParticles_Internal(particleType);
	}
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Unreliable, RplRcver.Server)]
	protected void RpcAsk_StopParticles(string particleType)
	{
		Rpc(RpcDo_StopParticles, particleType);
		StopParticles_Internal(particleType);
	}
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Unreliable, RplRcver.Broadcast)]
	protected void RpcDo_StopParticles(string particleType)
	{
		StopParticles_Internal(particleType);
	}
	//------------------------------------------------------------------------------------------------
	
	// #### INTERNAL METHODS ####
	
	//------------------------------------------------------------------------------------------------
	// particle start handler
	// manage a hashmap with active particles -> key is "particle type" and value is "particle entity"
	//! \param[in] particleType -> such as "smoke", "flame"
	protected void PlayParticles_Internal(string particleType)
	{
		if (activeParticles.Contains(particleType) && activeParticles.Get(particleType))
		{
			return;
		}

		ResourceName particlePath;
		if (!PARTICLE_PATHS.Find(particleType, particlePath))
		{
			PrintFormat("Unkown particle type %1", particleType, LogLevel.WARNING);
		}
		
		vector parentTransform[4];
		ParticleEffectEntitySpawnParams spawnParams = new ParticleEffectEntitySpawnParams();

		GetOwner().GetWorldTransform(parentTransform);
		spawnParams.Parent = GetOwner();
		spawnParams.UseFrameEvent = true;

		ParticleEffectEntity particle = ParticleEffectEntity.SpawnParticleEffect(particlePath, spawnParams);
		if (particle)
		{
			particle.Play();
			activeParticles.Set(particleType, particle);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// particle stop handler
	// manage a hashmap with active particles -> key is "particle type" and value is "particle entity"
	//! \param[in] particleType -> such as "smoke", "flame"
	protected void StopParticles_Internal(string particleType)
	{
		ParticleEffectEntity particle;
		if (activeParticles.Find(particleType, particle) && particle)
		{
			particle.Stop();
			activeParticles.Remove(particleType);
		}
	}
	
	// #### OVERRIDE METHODS ####
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (PARTICLE_PATHS.Count() == 0)
		{
			//PARTICLE_PATHS.Insert("smoke", SMOKE);
			PARTICLE_PATHS.Insert("smoke_Trail", SMOKE_TRAIL);
			PARTICLE_PATHS.Insert("flame_Left", FLAME_LEFT);
			PARTICLE_PATHS.Insert("flame_Right", FLAME_RIGHT);
		}
	}
}