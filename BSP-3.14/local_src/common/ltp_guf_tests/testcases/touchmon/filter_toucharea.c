#include "touchmon.h"

struct filter_toucharea_state {
	int is_touched_down;
	int is_touch_up_event;
	int is_x_valid;
	int is_y_valid;
	int last_x;
	int last_y;
};

#define X1	0
#define Y1	1
#define X2	2
#define Y2	3

int filter_toucharea_contains(int* area, int x , int y) {
	if(x >= min(area[X1], area[X2]) && x <= max(area[X1], area[X2]) &&
	   y >= min(area[Y1], area[Y2]) && y <= max(area[Y1], area[Y2]) ) {
			return TRUE;
	}
	
	return FALSE;
}

int filter_toucharea_evaluate(struct input_event* evt, int* params, int num_params, void** filter_state) {
	struct filter_toucharea_state* state = (struct filter_toucharea_state*) *filter_state;
	if(state == NULL) {
		// this is the first call. => allocate a state for this instance
		state = malloc(sizeof(struct filter_toucharea_state));
		if(state == NULL) {
			// error: NOMEM
			// TODO: TBROK
			fprintf(stderr, "COULD NOT ALLOCATE MEMORY!\n");
		}
		state->is_touched_down = FALSE;
		state->last_x = 0;
		state->last_y = 0;
		state->is_x_valid = FALSE;
		state->is_y_valid = FALSE;
		state->is_touch_up_event = FALSE;
		
		*filter_state = (void*) state;
	}

	// touch_down => start monitoring
	if(evt->type == EV_KEY && evt->code == BTN_TOUCH && evt->value == 1) {
		state->is_touched_down = TRUE;
		return FALSE;
	}
	
	// touch_up => stop monitoring
	if(evt->type == EV_KEY && evt->code == BTN_TOUCH && evt->value == 0) {
		state->is_touched_down = FALSE;
		state->is_touch_up_event = TRUE;
		return FALSE;
	}

	// sync event
	if(evt->type == EV_SYN && evt->code == 0) {
		if(state->is_touch_up_event || (state->is_touched_down && state->is_x_valid && state->is_y_valid) ) {
			// count event when this is a touchup or a touchmove event, then reset the state:
			state->is_x_valid = FALSE;
			state->is_y_valid = FALSE;
			state->is_touch_up_event = FALSE;
			if(num_params == 4 && filter_toucharea_contains(params, state->last_x, state->last_y)) {
				if(setting_verbose >= 3)
					fprintf(stderr, "counted toucharea event\n");
				return TRUE;
			}
			else
				return FALSE;
		}
		else {
			// coordinate-set is not valid, but we are synced => wait for another set
			state->is_x_valid = FALSE;
			state->is_y_valid = FALSE;
			state->is_touch_up_event = FALSE;
			return FALSE;
		}
	}
	// X/Y Event
	else if(evt->type == EV_ABS) {
		if(evt->code == ABS_X) {
			state->last_x = evt->value;
			state->is_x_valid = TRUE;
		}
		else if(evt->code == ABS_Y) {
			state->last_y = evt->value;
			state->is_y_valid = TRUE;
		}
	}
	
	return FALSE;
}

struct named_filter filter_toucharea = {
	.name = "TOUCHAREA",
	.descr = "     Monitor touchevents and count touches which happen in a specific area.\n\
     This filter takes four arguments: x1,y1,x2,y2 which specify the area in which touch \n\
     events are allowed. e.g. '-f TOUCHAREA,0,0,200,200' will count all touches that happen\n\
     in the upper left corner.\n\
     Note: this will count all valid X/Y events it receives, which may be several per actual\n\
     touch. Combine this filter with TOUCHPRESS to count only touch-up events which happen\n\
     in the specified area.",
	.filter_function = &filter_toucharea_evaluate,
	.num_params_min = 4,
	.num_params_max = 4,
};
