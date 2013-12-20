function LightOverlayToy::create(%this)
{
 
   // Initialize the toys settings.
    LightOverlayToy.moveSpeed = 50;
    LightOverlayToy.trackMouse = true;

	//LightOverlayToy.BackgroundBlendMode = 1;
	LightOverlayToy.BackgroundSrcBlendFactor = DST_COLOR;
	LightOverlayToy.BackgroundDstBlendFactor = ZERO;
	
	LightOverlayToy.OverlaySrcBlendFactor = SRC_ALPHA;
	LightOverlayToy.OverlayDstBlendFactor = ONE_MINUS_SRC_ALPHA;
	
	LightOverlayToy.FogSrcBlendFactor = SRC_ALPHA;
	LightOverlayToy.FogDstBlendFactor = ONE_MINUS_DST_ALPHA;

	LightOverlayToy.MaskSrcBlendFactor = SRC_ALPHA;
	LightOverlayToy.MaskDstBlendFactor = ONE_MINUS_SRC_ALPHA;
	
	//LightOverlayToy.PlanetoidBlendMode = 1;
	//LightOverlayToy.PlanetoidSrcBlendFactor = DST_COLOR;
	//LightOverlayToy.PlanetoidDstBlendFactor = ONE;
	
    // Add the custom controls.
    addNumericOption("Move Speed", 1, 150, 1, "setMoveSpeed", LightOverlayToy.moveSpeed, true, "Sets the linear speed to use when moving to the target position.");
    addFlagOption("Track Mouse", "setTrackMouse", LightOverlayToy.trackMouse, false, "Whether to track the position of the mouse or not." );
	
	
	addSelectionOption( "ZERO,ONE,DST_COLOR,ONE_MINUS_DST_COLOR,SRC_ALPHA,ONE_MINUS_SRC_ALPHA,DST_ALPHA,ONE_MINUS_DST_ALPHA,SRC_ALPHA_SATURATE", "BackgroundSrcBlendFactor", 5, "BackgroundSrcBlendFactor", true, "Sets the SrcBlendFactor for the Background." );
	addSelectionOption( "ZERO,ONE,SRC_COLOR,ONE_MINUS_SRC_COLOR,SRC_ALPHA,ONE_MINUS_SRC_ALPHA,DST_ALPHA,ONE_MINUS_DST_ALPHA", "BackgroundDstBlendFactor", 5, "BackgroundDstBlendFactor", true, "Sets the DstBlendFactor for the Background." );

	addSelectionOption( "ZERO,ONE,DST_COLOR,ONE_MINUS_DST_COLOR,SRC_ALPHA,ONE_MINUS_SRC_ALPHA,DST_ALPHA,ONE_MINUS_DST_ALPHA,SRC_ALPHA_SATURATE", "PlanetoidSrcBlendFactor", 5, "PlanetoidSrcBlendFactor", true, "Sets the SrcBlendFactor for the Planetoid." );
	addSelectionOption( "ZERO,ONE,SRC_COLOR,ONE_MINUS_SRC_COLOR,SRC_ALPHA,ONE_MINUS_SRC_ALPHA,DST_ALPHA,ONE_MINUS_DST_ALPHA", "BackgroundDstBlendFactor", "PlanetoidDstBlendFactor", 5, "PlanetoidDstBlendFactor", true, "Sets the DstBlendFactor for the Planetoid." );
	
 
      LightOverlayToy.reset();
}
   
function LightOverlayToy::destroy(%this)
{
   echo("@@@ LightOverlayToy::destroy function called");
}

function LightOverlayToy::reset(%this)
{
   SandboxScene.clear();
   
   SandboxScene.setGravity(0, 0);
   
   LightOverlayToy::createBackground();
      
   LightOverlayToy::createFog();
   
   LightOverlayToy::createMask();
   
   LightOverlayToy::createPlanetoid();
   
     // Create target.
    %this.createTarget();
    
    // Create sight.
    %this.createSight();
}


function LightOverlayToy::BackgroundBlendMode(%this , %value )
{
    LightOverlayToy.BackgroundBlendMode = %value;
	echo("LightOverlayToy.BackgroundBlendMode is now set to " SPC %value);
	
}

function LightOverlayToy::BackgroundSrcBlendFactor( %this , %value )
{
    LightOverlayToy.BackgroundSrcBlendFactor = %value;
	echo("LightOverlayToy.BackgroundBlendMode is now set to " SPC %value);
}

function LightOverlayToy::BackgroundDstBlendFactor( %this , %value )
{
    LightOverlayToy.BackgroundDstBlendFactor = %value;
	echo("LightOverlayToy.BackgroundDstBlendFactor is now set to " SPC %value);
}


function LightOverlayToy::PlanetoidBlendMode( %this , %value )
{
    //LightOverlayToy.PlanetoidBlendMode = %value;
	echo("LightOverlayToy.PlanetoidBlendMode is now set to " SPC %value);
}

function LightOverlayToy::PlanetoidSrcBlendFactor( %this , %value )
{
    //LightOverlayToy.PlanetoidSrcBlendFactor = %value;
	echo("LightOverlayToy.PlanetoidSrcBlendFactor is now set to " SPC %value);
}

function LightOverlayToy::PlanetoidDstBlendFactor( %this , %value )
{
    //LightOverlayToy.PlanetoidDstBlendFactor = %value;
	echo("LightOverlayToy.PlanetoidDstBlendFactor is now set to " SPC %value);
	
}

function LightOverlayToy::createFog( %this )  
{      
    // Create the sprite.  
    %object = new Sprite();  
     
    %object.Position = "0 0";  
    %object.SceneLayer = 2;  
    %object.Size = "90 65";  
  
    %object.Image = "LightOverlayToy:blackfog";  
      
	      echo("Fog Created");
    // This made the background unviewable, but the other objects blended
	// together like a hole in the darkness
    //%object.SrcBlendFactor = ZERO;  
    
    // This made the entire background visible? Objects still blend with it.
    //%object.SrcBlendFactor = ZERO;  
	  
    // Set up the blending factors  
	echo("%object.getSrcBlendFactor() is " SPC %object.getSrcBlendFactor());
	echo("%object.DstSrcBlendFactor() is " SPC %object.getDstBlendFactor());
    %object.SrcBlendFactor = LightOverlayToy.FogSrcBlendFactor;  
	%object.DstBlendFactor = LightOverlayToy.FogDstBlendFactor;  
    

              
    // Add the sprite to the scene.  
    SandboxScene.add( %object );   
     
}  

function LightOverlayToy::createBackground( %this )  
{      
    // Create the sprite.  
    %object = new Sprite();  
     
    %object.Position = "0 0";  
    %object.SceneLayer = 10;  
    %object.Size = "100 75";  
  
    %object.Image = "ToyAssets:JungleSky";  
      
	      
    // This made the background unviewable, but the other objects blended
	// together like a hole in the darkness
    //%object.SrcBlendFactor = ZERO;  
    
    // This made the entire background visible? Objects still blend with it.
    //%object.SrcBlendFactor = ZERO;  
	  
    // Set up the blending factors  
    //%object.BlendMode = LightOverlayToy.BackgroundBlendMode;  
	//%object.SrcBlendFactor = LightOverlayToy.BackgroundSrcBlendFactor;  
	//%object.DstBlendFactor = LightOverlayToy.BackgroundDstBlendFactor;  
    

              
    // Add the sprite to the scene.  
    SandboxScene.add( %object );   
     
}  
  
//-----------------------------------------------------------------------------  
  
function LightOverlayToy::createMask( %this )  
{      
    // Create the sprite.  
    %object = new Sprite();  
     
    %object.Position = "5 -5";  
	
	// If the scenelayer is above the "background" layer it acts as an object above
	// it should, as if the background and the things blending with it are underneath.
    %object.SceneLayer = 3;  
    %object.Size = 20;  
	
	  %object.setBlendAlpha( 0.1 );
	
	%object.SrcBlendFactor = LightOverlayToy.MaskSrcBlendFactor;  
	%object.DstBlendFactor = LightOverlayToy.MaskDstBlendFactor;  
    
  
    %object.Image = "ToyAssets:HollowArrow";  
          
    // Add the sprite to the scene.  
    SandboxScene.add( %object );      
}  

function LightOverlayToy::createSight( %this )
{
    // Create the sprite.
    %object = new Sprite();
    
    // Set the sight object.
    LightOverlayToy.SightObject = %object;
    
    // Set the static image.
    %object.Image = "ToyAssets:Crosshair2";

    // Set the blend color.
    %object.BlendColor = LightBlue;
    
    // Set the transparency.
    %object.setBlendAlpha( 0.5 );
    
	    %object.SrcBlendFactor = LightOverlayToy.OverlaySrcBlendFactor;  
	%object.DstBlendFactor = LightOverlayToy.OverlayDstBlendFactor;  
	
	%object.SceneLayer = 4;  
    
	// Set a useful size.
    %object.Size = 40;
    
    // Set the sprite rotating to make it more interesting.
    %object.AngularVelocity = -90;
    
    // Add to the scene.
    SandboxScene.add( %object );    
}

//-----------------------------------------------------------------------------

function LightOverlayToy::createTarget( %this )
{
    // Create the sprite.
    %object = new Sprite();

    // Set the target object.
    LightOverlayToy.TargetObject = %object;
    
    // Set the static image.
    %object.Image = "ToyAssets:Crosshair2";
    
    // Set the blend color.
    %object.BlendColor = Red;
         %object.SrcBlendFactor = LightOverlayToy.OverlaySrcBlendFactor;  
	%object.DstBlendFactor = LightOverlayToy.OverlayDstBlendFactor; 
    // Set a useful size.
    %object.Size = 30;
        
    %object.SceneLayer = 4;  		
		
    // Set the sprite rotating to make it more interesting.
    %object.AngularVelocity = 60;
    
    // Add to the scene.
    SandboxScene.add( %object );    
}

function LightOverlayToy::createPlanetoid( %this )
{

        // Create the planetoid.
        %object = new Sprite()
        {
            class = "Planetoid";
        };
        %object.Position = "20 -5";
        %object.Size = 25;
		%object.SceneLayer = 5;  
        %object.Image = "ToyAssets:Planetoid";
        %object.AngularVelocity = -5;

//%object.BlendMode = LightOverlayToy.PlanetoidBlendMode;  
//	%object.SrcBlendFactor = LightOverlayToy.PlanetoidSrcBlendFactor;  
//	%object.DstBlendFactor = LightOverlayToy.PlanetoidDstBlendFactor;  
		SandboxScene.add( %object );


}




//-----------------------------------------------------------------------------

function LightOverlayToy::setMoveSpeed( %this, %value )
{
    %this.moveSpeed = %value;
}

//-----------------------------------------------------------------------------

function LightOverlayToy::setTrackMouse( %this, %value )
{
    %this.trackMouse = %value;
}

//-----------------------------------------------------------------------------

function LightOverlayToy::onTouchDown(%this, %touchID, %worldPosition)
{
    // Set the target to the touched position.
    LightOverlayToy.TargetObject.Position = %worldPosition;
    
    // Move the sight to the touched position.
    LightOverlayToy.SightObject.MoveTo( %worldPosition, LightOverlayToy.moveSpeed );
}

//-----------------------------------------------------------------------------

function LightOverlayToy::onTouchMoved(%this, %touchID, %worldPosition)
{
    // Finish if not tracking the mouse.
    if ( !LightOverlayToy.trackMouse )
        return;
        
    // Set the target to the touched position.
    LightOverlayToy.TargetObject.Position = %worldPosition;
    
    // Move the sight to the touched position.
    LightOverlayToy.SightObject.MoveTo( %worldPosition, LightOverlayToy.moveSpeed );     
}
