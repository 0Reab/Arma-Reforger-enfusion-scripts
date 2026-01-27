[ComponentEditorProps(category: "ReabCustomModded", description: "Incendiary grenade script")]
class REAB_ContactComponentClass : ScriptComponentClass {}


class REAB_ContactComponent : ScriptComponent
{
	protected TimerTriggerComponent timerComp;
	protected ref array<IEntity> m_aNearbyCharacters;
	protected vector fragPos;
	
	protected bool ticking = false;
	protected bool running = false;
	protected bool doDamage = false;
	
	protected static const float CHECK_RADIUS = 0.01;
	protected static const float fragSize = 0.040;
	protected static const float entityEndTimer = 100.0;
	
	protected int damageTicks;
	
	[Attribute(defvalue: "0.15", desc: "Script tickrate in seconds", category: "Tickrate")]
	protected float limit;
	[Attribute(defvalue: "4", desc: "Do damage on tick X", category: "Damage tickrate")]
	protected float damageTickThreshold;
	[Attribute(defvalue: "20", desc: "Base damage value", category: "Damage")]
	protected float DAMAGE;
	protected float timer;
	protected float maxTimer;
	protected float globalTimer;
	protected float framerate;
	protected float groundHeight;
	protected float fragY;
	protected float deleteTimer = entityEndTimer + 100;
	
	
	protected void handleFps(float timeSlice)
	{
		framerate += timeSlice;
		Print(globalTimer);

		// lifetime manager
		if (globalTimer >= entityEndTimer)
		{
			running = false;
			globalTimer += limit;
			
			if (globalTimer >= deleteTimer)
			{
				IEntity owner = GetOwner();
				RplComponent rpl = RplComponent.Cast(owner.FindComponent(RplComponent));
				rpl.DeleteRplEntity(owner, false);
			}

			return;
		}
		
		// general tickrate
		if (framerate > limit)
		{
			running = true;
			framerate = 0;

			damageTicks += 1;
			if (damageTicks >= damageTickThreshold) // every 4th tick do dmg
			{
				doDamage = true;
				damageTicks = 0;
			}
			else
			{
				doDamage = false;
			}
		}
		else
		{
			running = false;
		}
	}

	
	protected void updateFragPos()
	{
		IEntity owner = GetOwner();
		fragPos = owner.GetOrigin();
		fragY = fragPos[1] - fragSize;
		groundHeight = SCR_TerrainHelper.GetTerrainY(fragPos, noUnderwater: true);		
	}
	
	
	protected int calculateDamage(vector charPos)
	{
		// 2.0 - 0.0 distance values
		float distance = vector.Distance(fragPos, charPos);
		
		float total = DAMAGE * Math.AbsFloat(distance - 2.0);

		int result = Math.Ceil(total);
		Print(result);
		return result;
	}
	
	
	// test
	protected void handleDamage(IEntity ent)
	{
		SCR_CharacterControllerComponent controller;
		ChimeraCharacter chimera;
		SCR_CharacterDamageManagerComponent damageMan;
		HitZone hitZone;
		
		controller = SCR_CharacterControllerComponent.Cast(ent.FindComponent(SCR_CharacterControllerComponent));
		chimera = ChimeraCharacter.Cast(ent);	
		
		bool isDead = controller.GetLifeState() == ECharacterLifeState.DEAD;
		
		if (!controller || !chimera || isDead)
			return;

		damageMan = SCR_CharacterDamageManagerComponent.Cast(chimera.GetDamageManager());
		if (!damageMan)
			return;
		
		hitZone = damageMan.GetRandomPhysicalHitZone();
		vector hitPosDirNorm[3];
		
		int currentDamage = calculateDamage(ent.GetOrigin());
		
		SCR_DamageContext context = new SCR_DamageContext(EDamageType.FIRE, currentDamage, hitPosDirNorm, ent, hitZone, null, null, -1, -1);
		
		damageMan.HandleDamage(context);	
	}
	
	
	protected void detonate()
	{
		//Print("detonate");
		timerComp.SetTimer(0.1);
		
		IEntity owner = GetOwner();
		
		// check people in sphere, and damage each one periodically
				
		if (doDamage)
		{
			//Print("do damage");
			checkForCharacters();
			m_aNearbyCharacters = {}; // could be bug here
		}
	}
	
	
	protected void checkForCharacters()
	{
		IEntity owner = GetOwner();
		m_aNearbyCharacters.Clear();
		owner.GetWorld().QueryEntitiesBySphere(fragPos, CHECK_RADIUS, QueryEntitiesCallbackMethod, null, EQueryEntitiesFlags.DYNAMIC | EQueryEntitiesFlags.WITH_OBJECT);
		//Print("There are " + m_aNearbyCharacters.Count() + " human entities");
		foreach (IEntity character : m_aNearbyCharacters)
		{
			// damage the character here
			handleDamage(character);
		}		
	}
	
	
	protected bool QueryEntitiesCallbackMethod(IEntity e)
	{
		if (!e || !ChimeraCharacter.Cast(e)) // only humans
			return false;

		m_aNearbyCharacters.Insert(e);
		return true;
	}
	
	
	override void OnPostInit(IEntity owner)
	{
		timerComp = TimerTriggerComponent.Cast(owner.FindComponent(TimerTriggerComponent));
		m_aNearbyCharacters = {};
		
		if (!timerComp)
		{
			//Print("no timer component ERROR");
			return;
		}
		maxTimer = timerComp.GetTimer();
		
		SetEventMask(owner, EntityEvent.FRAME);
		//Print("debug init OK 2");
	}
	
	
	override void EOnFrame(IEntity owner, float timeSlice)
	{	
		if (!Replication.IsServer())
        	return;
		
		handleFps(timeSlice);

		if (!running) { return; }
		
		timer = timerComp.GetTimer();
		ticking = timer < maxTimer;
		
		//Print(damageTicks);
		//Print(doDamage);
		
		if (ticking)
		{
			globalTimer += 1;
			updateFragPos();
			
			if (fragY <= groundHeight || timer <= 0.1)
			{
				detonate();
			}
		}
	}
}
