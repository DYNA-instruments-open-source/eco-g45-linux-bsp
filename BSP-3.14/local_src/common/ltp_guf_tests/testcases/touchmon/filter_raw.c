#include "touchmon.h"

int filter_raw_evaluate(struct input_event* evt, int* params, int num_params, void** state) {
	switch(num_params) {
		case 3:
			if(evt->value != params[2])
				return FALSE;
			/* Fallthrough */
		case 2:
			if(evt->code != params[1])
				return FALSE;
			/* Fallthrough */		
		case 1:
			if(evt->type != params[0])
				return FALSE;
	}
	return TRUE;
}

struct named_filter filter_raw = {
	.name = "RAW",
	.descr = "     Filter the raw event data against the given parameters. The filter takes one to three arguments: type,\n\
     code and value, where the latter two are optional. All events which match the given parameters will \n\
     pass this filter. E.g '-f RAW,1,1' will count all up/down events of the [ESC] key. \n\
     See include/linux/input.h in your kernel sources for event types and codes",
	.filter_function = &filter_raw_evaluate,
	.num_params_min = 1,
	.num_params_max = 3,
};
