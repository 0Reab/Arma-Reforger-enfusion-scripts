[ComponentEditorProps(category: "Movement/Character", description: "Jetpack script component for flying")]
class REAB_PowerupComponentClass : ScriptComponentClass {}

/*
------------------------------------------------------------------------------
TODO:
------------------------------------------------------------------------------
	+ add enum states for future proofing, lol
	+ get parent character of clothing
	+ define physics
	+ character movement input vector get method
	+ test if global character controller works, update docs if it does
	+ run script only server-side -> EOnFrame()
	+ jump button input for Z axis in movement vector method -> GetDirection()
	+ finish return vector -> GetDirection()
	+ apply impulse to character
	+ bug fix world relative inputs to char

------------------------------------------------------------------------------
FEATURES:
------------------------------------------------------------------------------
	- feather the impulse strength?
	- fuel mechanics?
	- particles, with replication for learning?
	- leaning character for "realistic" flying"?
	- different xy/z force values?
------------------------------------------------------------------------------
OPTIMIZE:
------------------------------------------------------------------------------
	- Implement tickrate system?
	- Redundant physics definition every frame in Fly() method?
------------------------------------------------------------------------------
BUGS:
------------------------------------------------------------------------------
------------------------------------------------------------------------------
*/

enum CharState
{
	JUMPING,
	GROUNDED
}


class REAB_PowerupComponent : ScriptComponent
{
	protected SCR_CharacterControllerComponent character;
	protected CharState state = CharState.GROUNDED;
	protected IEntity charEntity;
	
	protected float force = 25;
	
	protected bool jumpPressed = false;


	//---------------------------------------------------------------------------------------------------------------
	// handle enum states
	void UpdateState()
	{
		if (character.IsFalling())
			state = CharState.JUMPING;
		else
			state = CharState.GROUNDED;
	}
		
	//---------------------------------------------------------------------------------------------------------------
	// return vector of character movement inputs, and jump button for missing Z axis
	//! \param[out] movementInput
	vector GetDirection()
	{
		vector movementVector = character.GetMovementInput();
		
		if (!movementVector && !jumpPressed)
			return vector.Zero;
		
		// add jump to movement vector
		if (jumpPressed)
			movementVector += vector.Up;
		
		vector result = charEntity.VectorToParent(movementVector);
		
		//Print(result * force);
		return result;
	}
	
	//---------------------------------------------------------------------------------------------------------------
	// apply physics impulse according to movement direction
	//! \param[in] movementInput
	void Fly(vector movementInput)
	{
		if (!movementInput)
			return;
		
		Physics charPhysics = character.GetOwner().GetPhysics();

		if (!charPhysics)
		{
			Print("no char physics?");
			return;
		}
		else
		{
			charPhysics.ApplyImpulse(movementInput * force);
		}
	}
	
	//---------------------------------------------------------------------------------------------------------------
	void detectJumpInput() { jumpPressed = GetGame().GetInputManager().GetActionValue("CharacterSprint") > 0; }
	
	//---------------------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.FRAME);
	}

	//---------------------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (!Replication.IsServer())
			return;
		
		// character wearing this entity
		charEntity = owner.GetRootParent();

		if (!charEntity)
			return;
		
		if (!character || character.GetOwner() != charEntity)
		{
			character = SCR_CharacterControllerComponent.Cast(charEntity.FindComponent(SCR_CharacterControllerComponent));
			
			if (!character)
				return;
		}
		
		// handle enums
		UpdateState();
		detectJumpInput();
		
		switch (state)
		{
			case CharState.JUMPING:
				Fly(GetDirection());
				break;
			
			case CharState.GROUNDED:
				break;
		}
	}
}