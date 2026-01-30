[ComponentEditorProps(category: "REAB", description: "Drunk effects on player")]
class REAB_DrunkComponentClass : ScriptComponentClass {}

class REAB_DrunkComponent : ScriptComponent
{
	// Core components
	protected SCR_CharacterControllerComponent character;
	protected CharacterInputContext input;
	
	// Drunk state
	protected bool isDrunk = false;
	protected int drinkCount = 0;
	protected float drunkTimer = 0;
	
	// Tick system
	protected float tickTimer = 0;
	protected bool shouldTick = false;
	
	// Active effect state
	protected ref TurningEffect walkTurn;
	protected ref TurningEffect driveTurn;
	
	//------------------------------------------------------------------------------------------------
	void IsDrunk_REAB()
	{
		isDrunk = true;
		drinkCount++;
		Print(string.Format("[Drunk] Drinks: %1", drinkCount));
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.FRAME);
		character = SCR_CharacterControllerComponent.Cast(owner.FindComponent(SCR_CharacterControllerComponent));
		
		if (character)
			input = character.GetInputContext();
		
		Print("[Drunk] Component initialized");
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);
		
		if (!isDrunk)
			return;
		
		// Validate components
		if (!character || !input)
		{
			character = SCR_CharacterControllerComponent.Cast(owner.FindComponent(SCR_CharacterControllerComponent));
			if (character)
				input = character.GetInputContext();
			return;
		}
		
		// Update timers
		drunkTimer += timeSlice;
		tickTimer += timeSlice;
		
		// End drunk effect after 100 seconds
		if (drunkTimer >= 100)
		{
			isDrunk = false;
			drinkCount = 0;
			drunkTimer = 0;
			walkTurn = null;
			driveTurn = null;
			Print("[Drunk] Effect ended");
			return;
		}
		
		// Tick every 3 seconds
		if (tickTimer >= 3)
		{
			tickTimer = 0;
			shouldTick = !shouldTick;
		}
		
		// Apply ongoing effects
		if (walkTurn)
		{
			ApplyWalkTurn();
			return;
		}
		
		if (driveTurn)
		{
			ApplyDriveTurn();
			return;
		}
		
		// Apply new random effects on tick
		if (shouldTick)
		{
			ApplyRandomEffects();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ApplyRandomEffects()
	{
		int roll = Math.RandomInt(0, 10);
		float drunkLevel = drinkCount * 0.1;
		
		// Random lean
		float leanAmount = Math.RandomFloatInclusive(-1.0, 1.0) * drunkLevel;
		input.SetLean(leanAmount);
		
		// Random speed/stance
		character.SetDynamicSpeed(Math.RandomFloat(0, 1));
		character.SetDynamicStance(Math.RandomFloat(0, 1));
		
		// Random drop
		if (roll == 1 && character.IsPlayerControlled())
			character.ActionDropItem();
		
		// Start walking turn
		int threshold = Math.Max(10 - drinkCount, 1);
		if (roll >= threshold && !IsInVehicle())
		{
			walkTurn = new TurningEffect();
			walkTurn.duration = Math.RandomIntInclusive(25, 200);
			walkTurn.amount = Math.RandomFloatInclusive(-0.05, 0.05) * drunkLevel;
			Print(string.Format("[Drunk] Walk turn started: duration=%1, amount=%2", walkTurn.duration, walkTurn.amount));
		}
		
		// Start driving swerve
		if (roll >= 5 && IsInVehicle())
		{
			driveTurn = new TurningEffect();
			driveTurn.duration = Math.RandomIntInclusive(25, 200);
			driveTurn.amount = Math.RandomFloatInclusive(-0.25, 0.25) * drunkLevel;
			Print(string.Format("[Drunk] Drive swerve started: duration=%1, amount=%2", driveTurn.duration, driveTurn.amount));
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ApplyWalkTurn()
	{
		if (IsInVehicle())
		{
			walkTurn = null;
			return;
		}
		
		float currentAngle = input.GetAimingAngles()[0];
		float targetAngle = currentAngle + walkTurn.amount;
		
		input.SetLean(walkTurn.amount);
		character.SetHeadingAngle(targetAngle, true);
		
		// Fall if turning too hard near start
		float drunkLevel = drinkCount * 0.1;
		float fallThreshold = 0.05 * drunkLevel * 0.9;
		if (Math.AbsFloat(walkTurn.amount) >= fallThreshold && walkTurn.duration >= 170)
		{
			Print("[Drunk] Falling from turn");
			character.Ragdoll();
		}
		
		walkTurn.duration--;
		if (walkTurn.duration <= 0)
		{
			walkTurn = null;
			Print("[Drunk] Walk turn ended");
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ApplyDriveTurn()
	{
		if (!IsInVehicle())
		{
			driveTurn = null;
			return;
		}
		
		IEntity vehicle = GetOwner().GetRootParent();
		if (!vehicle)
		{
			driveTurn = null;
			return;
		}
		
		SCR_CarControllerComponent car = SCR_CarControllerComponent.Cast(vehicle.FindComponent(SCR_CarControllerComponent));
		if (!car)
		{
			driveTurn = null;
			return;
		}
		
		VehicleWheeledSimulation sim = car.GetWheeledSimulation();
		if (!sim)
		{
			driveTurn = null;
			return;
		}
		
		float currentSteering = sim.GetSteering();
		sim.SetSteering(currentSteering + driveTurn.amount);
		
		driveTurn.duration--;
		if (driveTurn.duration <= 0)
		{
			driveTurn = null;
			Print("[Drunk] Drive swerve ended");
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsInVehicle()
	{
		SCR_ChimeraCharacter chimera = SCR_ChimeraCharacter.Cast(GetOwner());
		if (chimera)
			return chimera.IsInVehicle();
		
		return false;
	}
}

//------------------------------------------------------------------------------------------------
// Helper class to track turning effects
class TurningEffect
{
	int duration;
	float amount;
}