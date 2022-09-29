# VEX V5 Auto Picker
This tool is used to select autonomous modes graphically on the VEX V5 brain.

Examples of how to use this library in your code can be found in src/main.cpp

Due to the changing VEX game each year, a separate library exists for each game. This repository is for Spin Up. Therefore, updating your pros project in the future won't pull a new field image, you will need to remove the previous library and add the new game's library.


## Usage

## Autonomous List
The autonomous 
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