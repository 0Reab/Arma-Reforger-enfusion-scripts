[ComponentEditorProps(category: "Movement/Character", description: "Jetpack script component for flying")]
class REAB_JetpackFlyingComponentClass : ScriptComponentClass {}

/*
------------------------------------------------------------------------------
TODO:
------------------------------------------------------------------------------
	+ add enum states for future proofing, lol
	+ get parent character of clothing
	+ define physics
	+ character movement input vector get method
	+ test if global character controller works, update docs if it does
	x run script only server-side -> EOnFrame(); has to be both client & serv
	+ jump button input for Z axis in movement vector method -> GetDirection()
	+ finish return vector -> GetDirection()
	+ apply impulse to character
	+ bug fix world relative inputs to char
	+ bug fix ApplyImuplse now works, jetpack is enabled on equip
	+ physics has to be on character entity - child to parent does not work
	+ split in two components, manager and phys handler kidna
	+ add this script component to human base prefab (not just US base)
	+ flame particle not turning off, call turn off at grounded too
	+ handle particles properly
	+ document new particle methods
------------------------------------------------------------------------------
FEATURES:
------------------------------------------------------------------------------
	- feather the impulse strength?
	- fuel mechanics?
	- particles, with replication for learning?
	- leaning character for "realistic" flying"?
	- different xy/z force values?
	- enable shooting while flying?
	- add sound to jetpack?
------------------------------------------------------------------------------
OPTIMIZE:
------------------------------------------------------------------------------
	+ Redundant physics definition every frame in Fly() method?
		(returning early if no movement input)
		and defined only on first method call

	- Implement tickrate system? prolly no need
------------------------------------------------------------------------------
BUGS:
------------------------------------------------------------------------------
	- flame particle not replicated on other clients by default like smoke
	- do i need on delete method?
	
------------------------------------------------------------------------------
*/

enum CharState
{
	JUMPING,
	GROUNDED
}


class REAB_JetpackFlyingComponent : ScriptComponent
{

	protected ref map<string, ParticleEffectEntity> activeParticles = new map<string, ParticleEffectEntity>();
	protected static ref map<string, ResourceName> PARTICLE_PATHS = new map<string, ResourceName>();

	protected static ResourceName SMOKE_PARTICLE_PATH = "{AEA751F0BE7FE821}Particles/Vehicle/Vehicle_smoke_car_exhaust_damage.ptc";
	protected static ResourceName FLAME_PARTICLE_PATH = "{A8D906F1965C5277}Particles/Weapon/Custom_Jetpack_Trail_M229.ptc";

	protected SCR_CharacterControllerComponent character;
	protected CharState state = CharState.GROUNDED;
	protected Physics charPhysics;
	protected IEntity charEntity;

	protected float force = 25;
	protected bool jumpPressed = false;
	protected bool jetpackPowerupWorn = false;


	//------------------------------------------------------------------------------------------------
	// particle start handler
	// manage a hashmap with active particles -> key is "particle type" and value is "particle entity"
	//! \param[in] particleType -> such as "smoke", "flame"
	void PlayParticles_REAB(string particleType)
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
	void StopParticles_REAB(string particleType)
	{
		ParticleEffectEntity particle;
		if (activeParticles.Find(particleType, particle) && particle)
		{
			particle.Stop();
			activeParticles.Remove(particleType);
		}
	}	

	//------------------------------------------------------------------------------------------------
	// handle enum states
	protected void UpdateState()
	{
		if (character.IsFalling())
			state = CharState.JUMPING;
		else
			state = CharState.GROUNDED;
	}

	//------------------------------------------------------------------------------------------------
	// return vector of character movement inputs, and jump button for missing Z axis
	//! \param[out] movementInput
	protected vector GetDirection()
	{
		//return vector.Up;
		vector movementVector = character.GetMovementInput();

		if (!movementVector && !jumpPressed)
		{
			StopParticles_REAB("flame");
			return vector.Zero;
		}

		// add jump to movement vector
		if (jumpPressed)
			movementVector += vector.Up;

		vector result = charEntity.VectorToParent(movementVector);

		//Print(result * force);
		PlayParticles_REAB("flame");
		return result;
	}

	//------------------------------------------------------------------------------------------------
	// apply physics impulse according to movement direction
	//! \param[in] movementInput
	protected void Fly(vector movementInput)
	{
		if (!movementInput)
			return;

		if (!charPhysics)
		{
			charPhysics = character.GetOwner().GetPhysics();
			return;
		}
		else
		{
			charPhysics.ApplyImpulse(movementInput * force);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void DetectJumpInput() { jumpPressed = GetGame().GetInputManager().GetActionValue("CharacterSprint") > 0; }

	// could refactor this dumbass code later
	
	//------------------------------------------------------------------------------------------------
	// set jetpack functionality ON
	bool Enable_REAB() { jetpackPowerupWorn = true; return true; }

	//------------------------------------------------------------------------------------------------
	// set jetpack functionality ON
	bool Disable_REAB() { jetpackPowerupWorn = false; return false; }

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (PARTICLE_PATHS.Count() == 0)
		{
			PARTICLE_PATHS.Insert("smoke", SMOKE_PARTICLE_PATH);
			PARTICLE_PATHS.Insert("flame", FLAME_PARTICLE_PATH);
		}

		SetEventMask(owner, EntityEvent.FRAME);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (!jetpackPowerupWorn)
			return;

		// character wearing this entity

		charEntity = owner;//.GetRootParent();

		if (!charEntity)
		{
			return;
		}

		if (!character || character.GetOwner() != charEntity)
		{
			character = SCR_CharacterControllerComponent.Cast(charEntity.FindComponent(SCR_CharacterControllerComponent));
			if (!character)
				return;
		}

		// handle states

		UpdateState();
		DetectJumpInput();

		switch (state)
		{
			case CharState.JUMPING:
				Fly(GetDirection());
				break;

			case CharState.GROUNDED:
				StopParticles_REAB("flame");
				break;
		}
	}
}
