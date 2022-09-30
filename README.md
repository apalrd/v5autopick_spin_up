# VEX V5 Auto Picker
This tool is used to select autonomous modes graphically on the VEX V5 brain.

Examples of how to use this library in your code can be found in src/main.cpp

Due to the changing VEX game each year, a separate library exists for each game. This repository is for Spin Up. Therefore, updating your pros project in the future won't pull a new field image, you will need to remove the previous library and add the new game's library.


## Usage
Construct an autonomous list as described below. Call `auto_picker()` with a pointer and length to initialize the auton picker on screen. If you would like to clear the picker, call `auto_clean()` to clear up graphical objects. This does not reset the selected auton and auton may still be called after clean is run.

```c
/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */
void initialize() 
{
	printf("Initialize!\n");
	/* Call Auto Picker to initialize it
	 * Once this is called the picker will remain active until cleaned
	 */
	auto_picker(auto_list, sizeof(auto_list)/sizeof(auto_routine_t));
}
```
Then, call `auto_run()` to dispatch the most recently selected program.
```c
/**
 * Runs the user autonomous code....
 */
void autonomous() 
{
	/* Run slected autonomous program */
	auto_run();
}
```
## Autonomous List
The autonomous functions must be defined with the following prototype (arguments) with C linkage:

```c
/* Auto function example - the arguments must be exactly as shown
 * color is defined as:
 * AUTO_COLOR_RED = -1
 * AUTO_COLOR_SKILLS = 0
 * AUTO_COLOR_BLUE = 1
 * so you can multiply numbers by color to invert the routine if necessary
 * 
 * Position is one of:
 * AUTO_POS_1
 * AUTO_POS_2
 * AUTO_POS_SKILLS
 * It may be useful to you
 */
void auto_skills_1(auto_color_t color,auto_pos_t pos)
{
	printf("I am Auto Skills 1, color is %d, pos is %d\n",color,pos);
    /* More autonomous code */
}
```
Then, the routines must be defined in a list:
```c
/* List of autonomous routines
 * It must be defined exactly like this
 * Each entry in the list has 3 objects:
 *  - the function to call (must have the color and position arguments)
 *  - the position for which it is valid (SKILLS, POS_1, or POS_2)
 *  - the name of the routine (shown on screen)
 */
const auto_routine_t auto_list[] =
{
	/* Robot skills options */
	{ auto_skills_1, AUTO_POS_SKILLS, "Skills Center Goal"},
	{ auto_skills_2, AUTO_POS_SKILLS, "Skills Side Goal"},
	{ auto_skills_3, AUTO_POS_SKILLS, "Skills Something Else"},
	{ auto_skills_4, AUTO_POS_SKILLS, "Skills Fun"},
	/* Match autos */
	{ auto_match_p1_1, AUTO_POS_1, "Match P1 Color"},
	{ auto_match_p1_2, AUTO_POS_1, "Match P1 Neutral"},
	{ auto_match_p2_1, AUTO_POS_2, "Match P2 Color"},
	{ auto_match_p2_2, AUTO_POS_2, "Match P2 Neutral"},
	{ auto_match_p2_3, AUTO_POS_2, "Match P2 Center"},
};
```
## Pre-Auto Function
A pre-auton function is optional. If used, it will replace the last program slot. The button will change color while the pre-auton routine is executing. This may be used to initialize inertial states.

Define a pre-auto function using the following prototype:
```c
/* Optional Pre-Auto function
 * This function performs any preparations you need prior to running Autonomous
 * It is selected by pressing the lowest button in the selector list
 * The currently selected color and position are available like a match auto
 * The selection on screen will show RED until the pre-auto function returns
 */
void pre_auto(auto_color_t color,auto_pos_t pos)
{
	printf("Entering Pre-Auto\n");
	/* Wait for fingers to get off */
	pros::Task::delay(500);
	/* Calibrate the IMU */
	pros::Task::delay(2000);
	printf("Exiting Pre-Auto\n");
}
```
Then pass the pre-auto function during `initialize()`:
```c
/* If you'd like to use a pre-auto function, you must define it right after auto_picker
 * If the user has already clicked a position, clicking on another
 * position will cause the pre-auto entry to become available
 */
auto_pre_auto(pre_auto);
```
## Getters
You may get the most recently selected position, color, and index into the list at any time, even after calling clean. This may be used in driver control.
```c
/* Get last selected color:
 * RED = 1
 * SKILLS = 0 (or none selected)
 * BLUE = -1
 */
auto_color_t auto_get_color();
/* Get last selected position
 * SKILLS = 0 (or none selected)
 * POS_1 = 1
 * POS_2 = 2
 */
auto_pos_t auto_get_pos();
/* Get last selected index into auto array (or -1 if none) */
int auto_get_active();
```
## Installation with PROS
To add this library to your PROS project:

Download the latest release zip file (i.e. `autopick_spin_up@1.1.0.zip`) from Github's release page into your project folder

Add the zip file to pros's local template archive (using the filename of the version you downloaded):
```bash
pros conduct fetch autopick_spin_up@1.1.0.zi
```

Apply the library to your PROS project
```bash
pros conduct apply autopick_spin_up
```

You should now delete the zip file.