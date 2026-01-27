[ComponentEditorProps(category: "REAB", description: "Drunk effects on player")]
class REAB_DrunkComponentClass : ScriptComponentClass {}


// add tickrate


class REAB_DrunkComponent : ScriptComponent
{
	protected SCR_CharacterControllerComponent character;
	protected CharacterInputContext input;
	protected SCR_CarControllerComponent car;
	protected VehicleWheeledSimulation vehicle;
	
    protected float baseTimer;
	protected float effectDuration;
	protected float tickrate;
	
	protected float chanceValue_FALL = 0.05;
	protected float chanceValue_DRIVE = 0.25;
	protected float chanceValue_LEAN = 1;
	
	protected bool ontick = false;
    protected bool isDrunk = false;
	
	protected static int DELAY = 3;
	
	protected int drinksAmount = 0;
	protected int actionChance;
	protected int doTurning;
	protected int doTurning_driving;
	protected float actionInput;
	protected float change_walking;
	protected float change_driving;

	
	//------------------------------------------------------------------------------------------------
	void IsDrunk_REAB()
	{
		isDrunk = true;
		drinksAmount += 1;

		// increase effects strength according to amount of drinks
		
		chanceValue_FALL *= drinksAmount / 10;
		chanceValue_LEAN *= drinksAmount / 10;
		chanceValue_DRIVE *= drinksAmount / 10;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.FRAME);

        character = SCR_CharacterControllerComponent.Cast(owner.FindComponent(SCR_CharacterControllerComponent));
		
		if (character)
			input = character.GetInputContext();

		Print("DEBUG INIT");
	}
		
	//------------------------------------------------------------------------------------------------
    void HandleTime(float timeSlice)
    {
		if (isDrunk && baseTimer >= 100) // in seconds
		{
			Print("drunk state finished");
			
			baseTimer = 0;
			effectDuration = 0;
			tickrate = 0;
			
			isDrunk = false;	
			drinksAmount = 0;
			return;
		}
		
		baseTimer += timeSlice;
		effectDuration += timeSlice;
		tickrate += timeSlice;
		
		if (tickrate >= DELAY)
		{
			tickrate = 0;
			ontick = !ontick; // switch
		}
    }
	
	//------------------------------------------------------------------------------------------------
    void Drunk_Speed()
    {
		character.SetDynamicSpeed(actionInput);
    }
	
	//------------------------------------------------------------------------------------------------
    void Drunk_Lean()
    {
        input.SetLean(Math.RandomFloatInclusive(-chanceValue_LEAN, chanceValue_LEAN));
    }
	
	//------------------------------------------------------------------------------------------------
    void Drunk_Drop()
    {
		if (actionChance == 1 && character.IsPlayerControlled())
        	character.ActionDropItem();
    }
	
	//------------------------------------------------------------------------------------------------
    void Drunk_Stance()
    {
        character.SetDynamicStance(actionInput);
    }

	//------------------------------------------------------------------------------------------------
    void Drunk_Move()
    {
		if (inVehicle())
			return;
		
		if (doTurning)
		{
			float currentAngle = input.GetAimingAngles()[0] / 3;
			
			//Print(change_walking);
			
			float targetAngle = input.GetAimingAngles()[0] + change_walking;
			
			input.SetLean(targetAngle);
			character.SetHeadingAngle(targetAngle, true);
			
			float fall_chance = chanceValue_FALL * 0.90;
			Print(fall_chance);
			Print(change_walking);
			
			if (!inVehicle() && Math.AbsFloat(change_walking) >= fall_chance && doTurning <= 30)
			{
				Print("Drunk_Move -> Ragdoll");
				character.Ragdoll();
			}
			
			doTurning -= 1;
			return;
		}
			
		if (actionChance >= 10 - drinksAmount) // could be bug here
		{
			doTurning = Math.RandomIntInclusive(25,200); // duration for method calls
			change_walking = Math.RandomFloatInclusive(-chanceValue_FALL, chanceValue_FALL); // amount
		}
	}
	
	//------------------------------------------------------------------------------------------------	
	void Drunk_Fall()
	{
		if (actionChance == 0 && !inVehicle())
		{
			//Print("Drunk_Fall");
			// if character.iscontrllled or smt
			//character.Ragdoll();
		}
	}

	//------------------------------------------------------------------------------------------------
	void Drunk_Drive()
	{
		if (!inVehicle())
			return;
		
		if (doTurning_driving)
		{
			if (!car)
			{
				car = SCR_CarControllerComponent.Cast(GetOwner().GetRootParent().FindComponent(SCR_CarControllerComponent));
				return;
			}
			
			if (!vehicle)
			{
				vehicle = car.GetWheeledSimulation(); // or this? GetSimulation();
				return;
			}
			
			float current_steer = vehicle.GetSteering();
			
			vehicle.SetSteering(change_driving + current_steer); //Math.RandomFloatInclusive(-1,1));
			doTurning_driving -= 1;
			return;
		}
		
		if (actionChance >= 5)
		{
			if (doTurning_driving)
				return;
			doTurning_driving = Math.RandomIntInclusive(25,200); // duration for method calls
			change_driving = Math.RandomFloatInclusive(-chanceValue_DRIVE, chanceValue_DRIVE); // amount
			Print(change_driving);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	bool inVehicle()
	{
		SCR_ChimeraCharacter chimera = SCR_ChimeraCharacter.Cast(GetOwner());
		if (chimera)
			return chimera.IsInVehicle();
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
        super.EOnFrame(owner, timeSlice);

        if (!isDrunk)
        {
			//Print("not drunk");
            return; // for debugging temporary
        }

        if (!character || !input)
        {
			Print("no character || input");
            character = SCR_CharacterControllerComponent.Cast(owner.FindComponent(SCR_CharacterControllerComponent));
			input = character.GetInputContext();
            return;
        }

        HandleTime(timeSlice);
		
		if (doTurning)
		{
			Drunk_Move();
			return;
		}
		
		if (doTurning_driving)
		{	
			Drunk_Drive();
			return;
		}
		
		if (!ontick)
		{
			return;
		}
		
		actionInput = Math.RandomFloat(0,1);
		actionChance = Math.RandomInt(0, 10);
		
		Drunk_Drive();
		Drunk_Fall();
		Drunk_Stance();
		Drunk_Speed();
		Drunk_Drop();
		Drunk_Move();
	
		/*	
		Drunk_Fall();
		Drunk_Stance();
		Drunk_Speed();
		Drunk_Drop();
		Drunk_Move();
		*/     
	}
}
