[ComponentEditorProps(category: "ReabCustomModded", description: "Incendiary grenade script - multiplayer fixed")]
class REAB_ContactComponentClass : ScriptComponentClass {}

// no damage in vehicles
// entity lifetime deletes all existing instances
class REAB_ContactComponent : ScriptComponent
{
	protected RplComponent m_RplComponent;
	protected ref array<IEntity> m_aNearbyCharacters;
	protected vector fragPos;
	
	protected bool m_bIsDetonated = false;
	protected bool m_bGroundHit = false;
	protected bool m_bIsThrown = false;
	
	protected static const float CHECK_RADIUS = 1.7; // Increased from 0.01 - was too small
	protected static const float FRAG_SIZE = 0.040;
	protected static const float ENTITY_END_TIMER = 100.0;
	
	protected int m_iDamageTicks;
	protected float m_fFrameTimer;
	protected float m_fGlobalTimer;
	protected float m_fGroundHeight;
	
	[Attribute(defvalue: "0.15", desc: "Script tickrate in seconds", category: "Tickrate")]
	protected float m_fTickRate;
	
	[Attribute(defvalue: "4", desc: "Do damage every X ticks", category: "Damage tickrate")]
	protected int m_iDamageTickThreshold;
	
	[Attribute(defvalue: "20", desc: "Base damage value", category: "Damage")]
	protected float m_fBaseDamage;
	
	[Attribute(defvalue: "13.0", desc: "Duration of damage effect after detonation (seconds)", category: "Damage")]
	protected float m_fDamageDuration;
	
	[Attribute(defvalue: "2.0", desc: "Fuse delay after ground impact (seconds)", category: "Detonation")]
	protected float m_fFuseDelay;
	
	protected float m_fDetonationTime = -1;
	protected float m_fGroundImpactTime = -1;
	
	//------------------------------------------------------------------------------------------------
	// RPC method to handle damage on server
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_Detonate()
	{
		// Security check - only allow detonation once
		if (m_bIsDetonated)
			return;
		
		// Additional check - must be thrown first
		if (!m_bIsThrown)
			return;
			
		m_bIsDetonated = true;
		m_fDetonationTime = m_fGlobalTimer;
		
		// Broadcast detonation to all clients for visual effects if needed
		Rpc(RpcDo_Detonate);
	}
	
	//------------------------------------------------------------------------------------------------
	// Broadcast detonation event to clients (for effects, sounds, etc.)
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_Detonate()
	{
		// Clients can handle visual/audio effects here
		// For now, just mark as detonated
		m_bIsDetonated = true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateFragmentPosition()
	{
		IEntity owner = GetOwner();
		if (!owner)
			return;
			
		fragPos = owner.GetOrigin();
		m_fGroundHeight = SCR_TerrainHelper.GetTerrainY(fragPos, noUnderwater: true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected int CalculateDamage(vector charPos)
	{
		// Distance-based damage calculation
		float distance = vector.Distance(fragPos, charPos);
		
		// Damage falloff: max damage at 0m, decreases with distance
		// Adjusted formula for better balance
		float damageMultiplier = Math.Max(0, 1.0 - (distance / CHECK_RADIUS));
		float totalDamage = m_fBaseDamage * damageMultiplier;
		
		//Print(totalDamage);
		return Math.Ceil(totalDamage);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void HandleDamage(IEntity ent)
	{
		if (!ent)
			return;
			
		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(
			ent.FindComponent(SCR_CharacterControllerComponent)
		);
		
		ChimeraCharacter chimera = ChimeraCharacter.Cast(ent);
		
		if (!controller || !chimera)
			return;
		
		// Check if already dead
		if (controller.GetLifeState() == ECharacterLifeState.DEAD)
			return;
		
		SCR_CharacterDamageManagerComponent damageMan = SCR_CharacterDamageManagerComponent.Cast(
			chimera.GetDamageManager()
		);
		
		if (!damageMan)
			return;
		
		// Get a random hit zone
		HitZone hitZone = damageMan.GetRandomPhysicalHitZone();
		if (!hitZone)
			return;
		
		// Calculate damage based on distance
		int damage = CalculateDamage(ent.GetOrigin());
		
		if (damage <= 0)
			return;
		
		// Create damage context
		vector hitPosDirNorm[3];
		hitPosDirNorm[0] = ent.GetOrigin(); // Hit position
		hitPosDirNorm[1] = vector.Up; // Direction
		hitPosDirNorm[2] = vector.Up; // Normal
		
		SCR_DamageContext context = new SCR_DamageContext(
			EDamageType.FIRE,
			damage,
			hitPosDirNorm,
			GetOwner(), // Instigator
			hitZone,
			null, // Collision info
			null, // Surface
			-1,
			-1
		);

		damageMan.HandleDamage(context);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void PerformDamage()
	{
		// Only run on server/authority
		if (!Replication.IsServer())
			return;
		
		IEntity owner = GetOwner();
		if (!owner)
			return;
		
		// Clear and repopulate character list
		m_aNearbyCharacters.Clear();
		
		// Query for nearby entities
		owner.GetWorld().QueryEntitiesBySphere(
			fragPos,
			CHECK_RADIUS,
			QueryEntitiesCallback,
			null,
			EQueryEntitiesFlags.ALL //DYNAMIC | EQueryEntitiesFlags.WITH_OBJECT // debug
		);
		
		// Apply damage to each character found
		foreach (IEntity character : m_aNearbyCharacters)
		{
			if (character)
				HandleDamage(character);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool QueryEntitiesCallback(IEntity e)
	{
		if (!e)
			return true;
		// find characters in vehicles too
		// Only accept character entities
		ChimeraCharacter character = ChimeraCharacter.Cast(e);
		if (!character)
			return true;
		
		m_aNearbyCharacters.Insert(e);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CheckGroundImpact()
	{
		IEntity owner = GetOwner();
		if (!owner)
			return;
		
		// Don't check for impact if not thrown yet
		if (!m_bIsThrown)
			return;
		
		vector pos = owner.GetOrigin();
		float currentHeight = pos[1];
		
		// Check if we've hit the ground
		if (currentHeight - FRAG_SIZE <= m_fGroundHeight && !m_bGroundHit)
		{
			m_bGroundHit = true;
			m_fGroundImpactTime = m_fGlobalTimer;
			
			// Don't detonate immediately - wait for fuse delay
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CheckFuseTimer()
	{
		// Only check fuse if we've hit the ground but not detonated yet
		if (!m_bGroundHit || m_bIsDetonated)
			return;
		
		// Calculate time since ground impact
		float timeSinceImpact = m_fGlobalTimer - m_fGroundImpactTime;
		
		// Check if fuse delay has elapsed
		if (timeSinceImpact >= m_fFuseDelay)
		{
			// On server, initiate detonation
			if (Replication.IsServer())
			{
				RpcAsk_Detonate();
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CheckIfThrown()
	{
		// Detect if grenade has been thrown by checking for physics/velocity
		IEntity owner = GetOwner();
		if (!owner || m_bIsThrown)
			return;
		
		Physics physics = owner.GetPhysics();
		if (!physics)
			return;
		
		// If grenade has significant velocity, it's been thrown
		vector velocity = physics.GetVelocity();
		float speed = velocity.Length();
		
		// Threshold: if moving faster than 1 m/s, consider it thrown
		if (speed > 1.0)
		{
			m_bIsThrown = true;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		// Get replication component
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		
		// Initialize character array
		m_aNearbyCharacters = new array<IEntity>();
		
		// Set up frame event
		SetEventMask(owner, EntityEvent.FRAME);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		// Only run simulation on server
		if (!Replication.IsServer())
			return;
		
		// Update global timer
		m_fGlobalTimer += timeSlice;
		
		// Check if entity lifetime is exceeded
		if (m_fGlobalTimer >= ENTITY_END_TIMER)
		{
			// Clean up entity after lifetime expires
			if (m_RplComponent && Replication.IsServer())
			{
				// RplComponent.DeleteRplEntity(owner, false); // debug
			}
			return;
		}
		
		// Update frame timer for tick rate
		m_fFrameTimer += timeSlice;
		
		// Update fragment position
		UpdateFragmentPosition();
		
		// Check if grenade has been thrown
		if (!m_bIsThrown)
		{
			CheckIfThrown();
		}
		
		// Check for ground impact before detonation
		if (!m_bIsDetonated && m_bIsThrown)
		{
			CheckGroundImpact();
		}
		
		// Check if fuse timer has elapsed after ground impact
		if (m_bGroundHit && !m_bIsDetonated)
		{
			CheckFuseTimer();
		}
		
		// Run damage tick logic after detonation
		if (m_bIsDetonated)
		{
			// Check if damage duration has expired
			float timeSinceDetonation = m_fGlobalTimer - m_fDetonationTime;
			if (timeSinceDetonation >= m_fDamageDuration)
			{
				// Stop dealing damage but keep entity alive until cleanup
				return;
			}
			
			// Apply damage at specified tick rate
			if (m_fFrameTimer >= m_fTickRate)
			{
				m_fFrameTimer = 0;
				m_iDamageTicks++;
				
				// Apply damage every N ticks
				if (m_iDamageTicks >= m_iDamageTickThreshold)
				{
					PerformDamage();
					m_iDamageTicks = 0;
				}
			}
		}
	}
}