#include "touchmon.h"

struct filter_touchpress_state {
	int was_touched;
};

int filter_touchpress_evaluate(struct input_event* evt, int* params, int num_params, void** filter_state) {
	struct filter_touchpress_state* state = (struct filter_touchpress_state*) *filter_state;
	
	if(state == NULL) {
		state = malloc(sizeof(struct filter_touchpress_state));
		
		if(state == NULL) {
			// TODO: TBROK => noMEM
			fprintf(stderr, "touchpress: NOMEM");
			return FALSE;
		}
		
		*filter_state = (void*)state;
	}

	// is sync event => count this touchdown
	if(evt->type == EV_SYN && state->was_touched) {
		state->was_touched = FALSE;
		if(setting_verbose >= 3)
			fprintf(stderr, "counted touchup\n");
		return TRUE;
	}
	
	// is touchup => wait for next sync to count it
	if(evt->type == EV_KEY && evt->code == BTN_TOUCH && evt->value == 0) {
		state->was_touched = TRUE;
		return FALSE;
	}
	else
		return FALSE;
}

struct named_filter filter_touchpress = {
	.name = "TOUCHPRESS",
	.descr = "     Let touch_up events be counted.",
	.filter_function = &filter_touchpress_evaluate,
	.num_params_min = 0,
	.num_params_max = 0,
};
