//
// Example of pentagram design by Ryan Nunn 
// 12th March 2002
//

class DisplayManager;


  //                            //
 // A generic map object class //
//                            //
class Map_object {
protected:
	// Shape and frame (should probably be inherited from an exult ShapeID like class)
	int		shape, frame;

	// Our coords in the world. This is what is used by the objects themselves,
	// collision detection, usecode and whatever. 
	int		x, y, z;

	// Bitwise no interpolation flags
	// Bit 0:  temporary 1 frame interpolation disable (autoreset)
	// Bit 1:  permanent interpolation disable
	// Bit 16: used internally by Display Manager to implement bit 0
	// When an object is created Bit 0 should be set
	int		no_interp;

	// Cached pointer to shape
	Shape	*cached_shape;

private:
	// The following are things only used by the display manager
	friend class DisplayManager;

	// Update of the following is automatically handled by DisplayManager

	// Screenspace coords of the shape (used for click detection)
	int		sx, sy;

	// The current and previous shape and frame (used for automatic dirty region detection)
	int		cshape, cframe;
	int		pshape, pframe;

	// Current and prev world coords of the shape (interpolate to/from these)
	int		cx, cy, cz;
	int		px, py, pz;

	// Is the shape dirty (automatically updated)
	// If position_dirty == true, the shape will need to be updated each
	// rendering frame.
	// If shape_dirty == true the shape needs to be updated on the first
	// rendering frame
	bool shape_dirty;
	bool position_dirty;

	// Used to update above (this in theory doesn't need to be done for fixed glob objects)
	// This is done after the world has been updated
	inline void update_current_and_prev() {
		// Update the prev
		pshape = cshape;
		pframe = cframe;
		px = cx;
		py = cy;
		pz = cz;

		// Now copy the current
		cshape = shape;
		cframe = frame;
		cx = x;
		cy = y;
		cz = z;

		// is the object dirty (i.e. changed shape, position changed)
		shape_dirty = cshape != pshape || cframe != pframe;
		position_dirty = cx != px || cy != py || cz != pz;
	}

	// Interpolated screenspace calc (factor 0 to 256)
	inline void calc_screenspace_interp(TileCorrd &camera, int factor) {

		// If no interp flag is set, disable interp
		if (no_interp) factor = 256;
		
		// TODO: Put calcs here
	};

	// Non interpolated screenspace calc
	inline void calc_screenspace(TileCorrd &camera) {

		// TODO: Write actual non interp function
		calc_screenspace_interp(TileCorrd &camera, 256);
	};


public:

	// Run the object... this would be used to run actor animations, handle
	// combat and whatever else an object or actor would want to do at a
	// predetermined time.
	//
	// Actual implimentation is doubtful to be like this :-)
	virtual void RunObject(int reason);
};

 
   //                                              //
  // The DisplayManager manages the Display List. //
 //  It also is used to do click detection       //
//                                              //
class DisplayManager {

	// The display list
	Display_list_type		display_list;

	// The dirty region
	Rectangle				dirty_region;

public:
	// factor is below is the interpolation factor. Set to 256 for no interp

	// Sets up a new list after the world has been updated
	void			SetupList(Map_object *objects, int factor);
	
	// Interpolates the list between world updates
	void			UpdateList(Map_object *objects, int	factor);

	// Find an object in the list
	Map_object *	FindObject(int x, int y);
	void			PaintObjects(Surface *surf);
};
DisplayManager	DispMan;


  //					    //
 // A generic world class. //
//                        //
class WorldManager {

public:
	// Run the world
	void			RunTheWorld();

	// Get all the objects
	Map_object *	GetObject();
};
WorldManager	TheWorld;


  //                         //
 // A generic gump manager. //
//                         //
class GumpManager {
public:
	// Paint the gumps
	void	PaintGumps(Surface *surf);

	// Handle click
	void	Clicked(int x, int y);
}
GumpManager		GumpMan;


  //                         //
 // A generic input manager //
//                         //
class InputManager {
public:
	HandleInput();
};
InputManager	InMan;


  //                     //
 // The usecode machine //
//                     //
class UsecodeMachine {
public:
	PaintText(Surface *surf);
	Run();
}
UsecodeMachine	Usecode;


// A flag to specify if we are interpolating
bool			interpolating = true;

// The animation rate, default to 50 MS (20 FPS)
int				ANIMATION_RATE = 50;

// TODO Make a Clock Class instead of doing timing like 
// how i've done below
//
// There are a number of very good reasons. 1 is to allow
// pausing.
//
// The timing method below will attempt to keep the 
// average timing to ANIMATION_RATE ms.
class GameClock {
}

// The program loop
int DoProgramLoop (void)
{
	// The time to run the next frame at
	static int	next_time = -1;

	// Time of the current frame
	int			time = GetCurrentTime();

	// Set the first time
	if (next_time == -1) next_time = time;

	// Handle the input
	InMan.HandleInput();

	// Run the world, if needed and run it as many times as we need to
	if (time >= next_time) while (time >= next_time) {

		// Run the world
		TheWorld.RunWorld();

		// Run the usecode
		Usecode.Run();
		
		// Update the next time to run
		next_time += ANIMATION_RATE;

		// Calc interp factor
		int factor = next_time - time;
		factor *= 256;
		factor /= ANIMATION_RATE;
		if (!interpolating) factor = 256;

		// Note we do need to resetup the list for every running of the world
		DispMan.SetupList(TheWorld.GetObjects(), factor);
	}
	else {
		// Calc interp factor
		int factor = next_time - time;
		factor *= 256;
		factor /= ANIMATION_RATE;
		if (!interpolating) factor = 256;

		// Update display list
		DispMan.UpdateList(TheWorld.GetObjects(), factor);
	}

	// Now paint everything
	DispMan.PaintObjects(DisplaySurface);
	GumpMan.PaintGumps(DisplaySurface);
	Usecode.PaintText(DisplaySurface);
}
