#include "touchmon.h"

int filter_sync_evaluate(struct input_event* evt, int* params, int num_params, void** state) {
	return (evt->type == 0) ? TRUE : FALSE;
}

struct named_filter filter_sync = {
	.name = "SYNC",
	.descr = "     Count only 'SYNC' events (these events contain no data and are used \n\
     to group events according to their content (e.g. x,y-coordinates of a touch-\n\
     event) e.g: '-f SYNC'",
	.filter_function = &filter_sync_evaluate,
	.num_params_min = 0,
	.num_params_max = 0,
};
