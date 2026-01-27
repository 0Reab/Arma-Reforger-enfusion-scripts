[ComponentEditorProps(category: "REAB_ScriptComponents/BaseCharacter", description: "Jetpack script component for flying")]
class REAB_JetpackFlyingComponentClass : ScriptComponentClass {}

/*
------------------------------------------------------------------------------
TODO:
------------------------------------------------------------------------------
------------------------------------------------------------------------------
FEATURES:
------------------------------------------------------------------------------
	- enable shooting while flying?
	- hover on current height mode?
------------------------------------------------------------------------------
BUGS:
------------------------------------------------------------------------------
	- do i need on delete method? test needed
	- i think particles are spawning every frame, nice one bro
------------------------------------------------------------------------------
*/

enum CharState
{
	JUMPING,
	GROUNDED
}


class REAB_JetpackFlyingComponent : ScriptComponent
{
	// character
	protected SCR_CharacterControllerComponent character;
	protected CharState state = CharState.GROUNDED;
	protected CharState statePrevious;
	protected Physics charPhysics;
	protected IEntity charEntity;

	// flying
	protected float force = 100;
	protected bool jumpPressed = false;
	protected bool jetpackPowerupWorn = false;
	
	// particles & audio
	protected REAB_JetpackParticlesComponent particles;
	protected REAB_JetpackSoundsComponent sounds;
	
	
	// #### PUBLIC METHODS ####
	//------------------------------------------------------------------------------------------------
	void Enable_REAB() { jetpackPowerupWorn = true; }
	void Disable_REAB() { jetpackPowerupWorn = false; }

	//------------------------------------------------------------------------------------------------
	// handle enum states
	protected void UpdateState()
	{
		if (character.IsFalling())
		{
			statePrevious = state;
			state = CharState.JUMPING;
		}
		else
		{
			statePrevious = state;
			state = CharState.GROUNDED;
		}
	}

	//------------------------------------------------------------------------------------------------
	// return vector of character movement inputs, and jump button for missing Z axis
	//! \param[out] movementInput
	protected vector GetDirection()
	{
		vector movementVector = character.GetMovementInput();

		if (!movementVector && !jumpPressed)
		{
			sounds.StopSound_REAB();	
			return vector.Zero;
		}

		// add jump to movement vector
		if (jumpPressed)
			movementVector += vector.Up;

		vector result = charEntity.VectorToParent(movementVector);
		
		sounds.PlaySound_REAB();//"SOUND_JETPACK_THRUST"
				
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
		
		float forceMod = character.GetDynamicSpeed(); // 0 to 1 float
		
		charPhysics.ApplyImpulse(movementInput * (force * forceMod));
	}
	
	// #### HELPER METHODS ####
	//------------------------------------------------------------------------------------------------
	protected void GetCharacterController(IEntity e) { character = SCR_CharacterControllerComponent.Cast(e.FindComponent(SCR_CharacterControllerComponent)); }
	//------------------------------------------------------------------------------------------------
	protected void GetParticlesComponent(IEntity e) { particles = REAB_JetpackParticlesComponent.Cast(e.FindComponent(REAB_JetpackParticlesComponent)); }
	//------------------------------------------------------------------------------------------------
	protected void GetSoundsComponent(IEntity e) { sounds = REAB_JetpackSoundsComponent.Cast(e.FindComponent(REAB_JetpackSoundsComponent)); }
	//------------------------------------------------------------------------------------------------
	protected void DetectJumpInput() { jumpPressed = GetGame().GetInputManager().GetActionValue("CharacterSprint") > 0; }
	//------------------------------------------------------------------------------------------------
	protected void SetDynamicSpeed(float speed) { if (state != statePrevious) { character.SetDynamicSpeed(speed); }} // based on first frame of character state change -> set speed to arg

	//------------------------------------------------------------------------------------------------
	protected void HandleParticles(vector movement)
	{
		if (movement != vector.Zero)
		{
			particles.PlayThrusterParticles_REAB(true);
			return;
		}
		
		particles.StopThrusterParticles_REAB(false);
	}
	
	// #### OVERRIDE METHODS ####
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		GetCharacterController(owner);
		GetParticlesComponent(owner);
		GetSoundsComponent(owner);
		
		charEntity = owner;
		
		SetEventMask(owner, EntityEvent.FRAME);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (!jetpackPowerupWorn)
			return;
		
		if (!character || !charEntity)
		{	
			GetCharacterController(owner);
			return;
		}
		
		if (!particles || !sounds)
		{
			GetParticlesComponent(owner);
			GetSoundsComponent(owner);
			return;
		}
		
		if (character.IsDead())
		{
			particles.StopAllParticles_REAB();
			sounds.StopSound_REAB();
			return;
		}

		UpdateState();
		DetectJumpInput();

		switch (state)
		{
			case CharState.JUMPING:
			
				vector direction = GetDirection();
				SetDynamicSpeed(0.25);
				Fly(direction);
				HandleParticles(direction);
				break;

			case CharState.GROUNDED:
			
				SetDynamicSpeed(1);
				particles.StopAllParticles_REAB();
				sounds.StopSound_REAB();
				break;
		}
	}
}
