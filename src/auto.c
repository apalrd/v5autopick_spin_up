/* Auto Picker library for PROS V5
 * Copyright (c) 2022 Andrew Palardy
 * This code is subject to the BSD 2-clause 'Simplified' license
 * See the LICENSE file for complete terms
 */
#include "pal/auto.h"
#include "pros/apix.h"
#include "display/lvgl.h"


/* System global variables */
static int active_mode;
static auto_pos_t active_pos = AUTO_POS_SKILLS;
static auto_color_t active_color = AUTO_COLOR_SKILLS;
static const auto_routine_t * local_auto_list;
static size_t local_auto_length;
static int active_auto;

/* Pre-Auto button function */
static auto_func_t local_pre_auto = NULL;

/* Mapping of active mode to active pos */
static const auto_pos_t active_pos_mode[] = 
{
	AUTO_POS_1,
	AUTO_POS_2,
	AUTO_POS_1,
	AUTO_POS_2,
	AUTO_POS_SKILLS
};

/* Mapping of active mode to active color */
static const auto_pos_t active_color_mode[] = 
{
	AUTO_COLOR_RED,
	AUTO_COLOR_RED,
	AUTO_COLOR_BLUE,
	AUTO_COLOR_BLUE,
	AUTO_COLOR_SKILLS
};

/* LV field image object */
lv_obj_t * fieldimg = NULL;

/* Number of buttons we can fit on the side of the screen at once - will rescale
 * If you have more than this many autos per position, the rest won't show up
 */
#define LIST_SIZE 8

/* List box to select program */
static lv_obj_t * list_btn[LIST_SIZE] = {0};

/* 5 buttons for starting positions (4 match, 1 skills) */
static lv_obj_t * btn_pos[5] = {0};
static const char * btn_label[5] = 
{
	"Red 1",
	"Red 2",
	"Blue 1",
	"Blue 2",
	"Skills"
};
static const lv_align_t btn_align[5] = 
{
	LV_ALIGN_IN_LEFT_MID,
	LV_ALIGN_IN_BOTTOM_MID,
	LV_ALIGN_IN_RIGHT_MID,
	LV_ALIGN_IN_TOP_MID,
	LV_ALIGN_CENTER
};

/* Styles */
static lv_style_t style_red_ina, style_red_act;
static lv_style_t style_blue_ina, style_blue_act;
static lv_style_t style_gold_ina, style_gold_act;
static lv_style_t style_list_ina, style_list_act;
static lv_style_t style_pa_ina, style_pa_act;

/* Styles associated with each starting position button in the inactive and active states */
static lv_style_t * const btn_styles[5][2] = 
{
	{&style_red_ina, &style_red_act},
	{&style_red_ina, &style_red_act},
	{&style_blue_ina, &style_blue_act},
	{&style_blue_ina, &style_blue_act},
	{&style_gold_ina, &style_gold_act}
};


/* List callback */
static lv_res_t btn_list_cb(lv_obj_t * btn)
{
	/* Find the pointer that matches the callback */
	int idx;
	for(idx = 0; idx < LIST_SIZE; idx++)
	{
		if(btn == list_btn[idx])
		{
			break;
		}
	}

	/* If pre-auto is enabled and we are in pre-auto mode, do that */
	if(local_pre_auto && (LIST_SIZE-1)==idx)
	{
		/* Set button style for pre-auto */
		lv_obj_set_style(list_btn[LIST_SIZE-1],&style_pa_act);
		/* Call pre-auto function */
		local_pre_auto(active_pos,active_color);
		/* Set button style back */
		lv_obj_set_style(list_btn[LIST_SIZE-1],&style_pa_ina);

		/* Exit function instead of processing as a list entry */
		return LV_RES_OK;
	}

	/* Limit list size if using pre-auto */
	int list_max = LIST_SIZE;
	if(local_pre_auto) list_max--;

	/* Toggle all of the button colors based on the selected button */
	for(int i = 0; i < list_max; i++)
	{
		/* Ignore buttons which are null,
		 * turn on button which matches idx
		 */
		if((list_btn[i]) && (i == idx))
		{
			/* Toggle on */
			lv_obj_set_style(list_btn[i],&style_list_act);
		}
		else if(list_btn[i])
		{
			/* Toggle off */
			lv_obj_set_style(list_btn[i],&style_list_ina);
		}
	}

	/* Determine active program to run:
	 * Go through the entire auto list to find ones matching the active pos
	 * Find the nth one in the list (matches idx)
	 * Find the i of that, as the active program 
	 */
	int valid_count = 0;
	for(int i = 0; i < local_auto_length; i++)
	{
		/* Does the position match? */
		if(local_auto_list[i].position == active_pos)
		{
			/* Check if active count matches idx */
			if(valid_count == idx)
			{
				/* Found what we are looking for, set it and break */
				active_auto = i;
				break;
			}
			/* Otherwise, increment valid_count */
			valid_count++;
		}
	}

	/* Return OK */
    return LV_RES_OK;
	
}
/* Position Button callback */
static  lv_res_t btn_pos_cb(lv_obj_t * btn)
{
    /* Find which index we are */
	int idx;
	for(idx = 0; idx < 5; idx++)
	{
		if(btn == btn_pos[idx])
		{
			break;
		}
	}
	active_mode = idx;
	/* Toggle off all buttons but toggle on ours */
	for(int i = 0; i < 5; i++)
	{
		if(i == idx)
		{
			/* Toggle on */
			lv_obj_set_style(btn_pos[i],btn_styles[i][1]);
		}
		else
		{
			/* Toggle off */
			lv_obj_set_style(btn_pos[i],btn_styles[i][0]);
		}
	}
	
	/* If the valid auto list was previously in existence, delete the buttons */
	for(int i = 0; i < LIST_SIZE; i++)
	{
		/* If object exists, delete it */
		if(list_btn[i])
		{
			lv_obj_del(list_btn[i]);
		}
		/* Clean up reference */
		list_btn[i] = NULL;
	}

	/* Convert active mode into a starting position and color */
	active_pos = active_pos_mode[active_mode];
	active_color = active_color_mode[active_mode];

	/* Determine how many valid routines there are for this starting position
	 * Also get an array of the pointers to their names
	 */
	int valid_autos = 0;
	char * valid_auto_names[LIST_SIZE] = {0};
	for(int i = 0; i < local_auto_length; i++)
	{
		if(local_auto_list[i].position == active_pos)
		{
			valid_auto_names[valid_autos] = local_auto_list[i].name;
			valid_autos++;
		}
	}

	/* Check if we have too many autos */
	if(valid_autos > LIST_SIZE)
	{
		printf("WARNING: Auto Picker has too many valid programs to fit on screen");
		valid_autos = LIST_SIZE;
	}
	/* The possible length is one less if pre-auto is being used */
	else if(local_pre_auto && (valid_autos > (LIST_SIZE - 1)))
	{
		printf("WARNING: Auto Picker has too many valid programs to fit on screen");
		valid_autos = LIST_SIZE - 1;
	}

	/* Determine how bit the buttons should be */
	int bwidth = lv_obj_get_width(lv_scr_act()) - 240;
	int bheight = lv_obj_get_height(lv_scr_act()) / LIST_SIZE;

	/* Create buttons for this list */
	for(int i = 0; i < valid_autos; i++)
	{
		/* Create button */
		list_btn[i] = lv_btn_create(lv_scr_act(), NULL);
		lv_btn_set_action(list_btn[i],LV_BTN_ACTION_CLICK,btn_list_cb);

		/* Set Style */
		lv_btn_set_style(list_btn[i],LV_BTN_STYLE_INA,&style_list_ina);
		lv_btn_set_style(list_btn[i],LV_BTN_STYLE_PR,&style_list_act);
		lv_btn_set_style(list_btn[i],LV_BTN_STYLE_REL,&style_list_ina);

		/* Size the button */
		lv_obj_set_size(list_btn[i], bwidth, bheight);

		/* Align button to top right offset by button number */
		lv_obj_align(list_btn[i],lv_scr_act(),LV_ALIGN_IN_TOP_RIGHT,0,i*bheight);

		/* Add label */
		lv_obj_t * label = lv_label_create(list_btn[i],NULL);
		lv_label_set_text(label,valid_auto_names[i]);
	}

	/* If the pre-auto button should be displayed, display it */
	if(local_pre_auto)
	{
		/* Create button */
		list_btn[LIST_SIZE-1] = lv_btn_create(lv_scr_act(), NULL);
		lv_btn_set_action(list_btn[LIST_SIZE-1],LV_BTN_ACTION_CLICK,btn_list_cb);

		/* Set Style */
		lv_btn_set_style(list_btn[LIST_SIZE-1],LV_BTN_STYLE_INA,&style_pa_ina);
		lv_btn_set_style(list_btn[LIST_SIZE-1],LV_BTN_STYLE_PR,&style_pa_act);
		lv_btn_set_style(list_btn[LIST_SIZE-1],LV_BTN_STYLE_REL,&style_pa_ina);

		/* Size the button */
		lv_obj_set_size(list_btn[LIST_SIZE-1], bwidth, bheight);

		/* Align button to top right offset by button number */
		lv_obj_align(list_btn[LIST_SIZE-1],lv_scr_act(),LV_ALIGN_IN_TOP_RIGHT,0,(LIST_SIZE-1)*bheight);

		/* Add label */
		lv_obj_t * label = lv_label_create(list_btn[LIST_SIZE-1],NULL);
		lv_label_set_text(label,"Pre-Auto Routine");
	}

    return LV_RES_OK;
}


void auto_picker(const auto_routine_t * list, size_t length)
{
    /* Save arguments to be used later */
    local_auto_list = list;
    local_auto_length = length;

	/* Draw image of the field */
    LV_IMG_DECLARE(field);
    fieldimg = lv_img_create(lv_scr_act(),0);
    lv_img_set_src(fieldimg, &field);
    lv_obj_align(fieldimg, 0, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_obj_set_size(fieldimg, 240, 240);

	/* Declare label for later */
	lv_obj_t * label;

	/* Create red style by copying default button style */
	lv_style_copy(&style_red_ina,&lv_style_btn_ina);
	style_red_ina.body.main_color=LV_COLOR_RED;
	style_red_ina.body.grad_color=LV_COLOR_RED;
	style_red_ina.body.border.color = LV_COLOR_BLACK;
	style_red_ina.body.border.width = 5;
	style_red_ina.text.color = LV_COLOR_BLACK;

	/* Create active style in slightly different colors */
	lv_style_copy(&style_red_act,&style_red_ina);
	style_red_act.body.border.color = LV_COLOR_WHITE;

	/* Copy blue style also */
	lv_style_copy(&style_blue_ina,&style_red_ina);
	style_blue_ina.body.main_color=LV_COLOR_BLUE;
	style_blue_ina.body.grad_color=LV_COLOR_BLUE;
	lv_style_copy(&style_blue_act,&style_blue_ina);
	style_blue_act.body.border.color = LV_COLOR_WHITE;

	/* Copy gold finally */
	lv_style_copy(&style_gold_ina,&style_red_ina);
	style_gold_ina.body.main_color=LV_COLOR_YELLOW;
	style_gold_ina.body.grad_color=LV_COLOR_YELLOW;
	lv_style_copy(&style_gold_act,&style_gold_ina);
	style_gold_act.body.border.color = LV_COLOR_WHITE;

	/* Create 5 buttons, 2 red and 2 blue, for each starting position, plus one for skills */
	for(int i = 0; i < 5; i++)
	{
		/* Create button */
		btn_pos[i] = lv_btn_create(lv_scr_act(), NULL);
		lv_btn_set_action(btn_pos[i],LV_BTN_ACTION_CLICK,btn_pos_cb);

		/* Set Style */
		lv_btn_set_style(btn_pos[i],LV_BTN_STYLE_INA,btn_styles[i][0]);
		lv_btn_set_style(btn_pos[i],LV_BTN_STYLE_PR,btn_styles[i][1]);
		lv_btn_set_style(btn_pos[i],LV_BTN_STYLE_REL,btn_styles[i][0]);

		/* Size the button */
		lv_obj_set_size(btn_pos[i], 66, 66);

		/* Align button to corner of the field */
		lv_obj_align(btn_pos[i],fieldimg,btn_align[i],0,0);

		/* Add label */
		label = lv_label_create(btn_pos[i],NULL);
		lv_label_set_text(label,btn_label[i]);
	}

	/* Style for program selection buttons (high contrast) */
	lv_style_copy(&style_list_ina,&style_gold_ina);
	style_list_ina.body.main_color=LV_COLOR_LIME;
	style_list_ina.body.grad_color=LV_COLOR_LIME;
	lv_style_copy(&style_list_act,&style_list_ina);
	style_list_act.body.border.color=LV_COLOR_WHITE;

	/* Style for pre-auto buttons (high contrast different colors) */
	lv_style_copy(&style_pa_ina,&style_list_ina);
	style_pa_ina.body.main_color=LV_COLOR_MAGENTA;
	style_pa_ina.body.grad_color=LV_COLOR_MAGENTA;
	lv_style_copy(&style_pa_act,&style_pa_ina);
	style_pa_act.body.main_color=LV_COLOR_ORANGE;
	style_pa_act.body.grad_color=LV_COLOR_ORANGE;
	style_pa_act.body.border.color=LV_COLOR_WHITE;


    /* Initially set active auto to -1 to indicate no selection */
    active_auto = -1;
}

/* Tell the picker we have a pre-auto function */
void auto_pre_auto(auto_func_t pre_auto)
{
	local_pre_auto = pre_auto;
}

/* Clean up all entities on the screen so it can be used by something else */
void auto_clean(void)
{
	/* Destroy everything in the selector list */
	for(int i = 0; i < LIST_SIZE; i++)
	{
		if(list_btn[i])
		{
			lv_obj_del(list_btn[i]);
		}
		list_btn[i] = NULL;
	}

	/* Destroy all 5 position buttons */
	for(int i = 0; i < 5; i++)
	{
		if(btn_pos[i])
		{
			lv_obj_del(btn_pos[i]);
		}
		btn_pos[i] = NULL;
	}
	
	/* Destroy the field */
	if(fieldimg)
	{
		lv_obj_del(fieldimg);
	}
	fieldimg = NULL;
}

/* Run the selected autonomous */
void auto_run(void)
{
    /* If active auto is negative, it's invalid */
    if(active_auto < 0)
    {
        return;
    }

    /* If active auto is greater than the size of the list, it's also invalid */
    if(active_auto >= local_auto_length)
    {
        return;
    }

    /* Get the pointer identified by the active auto, check it's valid, call it */
    auto_func_t function = local_auto_list[active_auto].function;
    if(function)
    {
        function(active_color,active_pos);
    }
	else
	{
		printf("ERROR: Auto picker tried to run program %s, but function pointer is invalid",local_auto_list[active_auto].name);
	}
}

/* Get last selected color (or skills if none) */
auto_color_t auto_get_color()
{
	return active_color;
}
/* Get last selected position (or skills if none) */
auto_pos_t auto_get_pos()
{
	return active_pos;
}
/* Get last selected index into auto array (or -1 if none) */
int auto_get_active()
{
	return active_auto;
}