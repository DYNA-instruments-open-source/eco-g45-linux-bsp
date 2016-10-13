#include "touchmon.h"

#include "filters.h"

/* apply all instantiated filters to the given event */
/* returns TRUE when the event has passed all filters, FALSE otherwise */
int apply_filters(struct input_event* evt) {
	struct filter_instance* current = filter_head;
	int passed;
	int ret;
	if(setting_filter_mode_or)
		passed = FALSE;
	else
		passed = TRUE;
	
	while(current != NULL) {
		if(current->filter->filter_function != NULL) {
			ret = current->filter->filter_function(evt, current->params, current->num_params, &current->state);
			if(setting_filter_mode_or) 
				passed |= ret;
			else
				passed &= ret;
				
			if(setting_verbose >= 4)
				fprintf(stderr, "filter function returned %i \n", ret);
		}
		current = current->next;
	}
	
	return passed;
}

/* instantiate the filter specified by arg.
   arg is a string of the form "<filtername>,<param1>,<param2>,..." */
int instantiate_filter(char* arg) {
	char* pos;
	int* params;
	int num_args = 0;
	int i;
	char* token;
	struct filter_instance* filter_inst = NULL;
	
	/* special option: */
	if(strncmp(arg, "mode_or", 7) == 0) {
		setting_filter_mode_or = TRUE;
		return 0;
	}
	
	/* parse normal filter names and options: */
	for(i = 0; filters_installed[i] != NULL; i++) {
		pos = strstr(arg, filters_installed[i]->name);
		if(pos == arg) {
			// filter found=> parse parameters
			if(filters_installed[i]->num_params_max > 0) {
				pos += strlen(filters_installed[i]->name);
				
				params = calloc(filters_installed[i]->num_params_max, sizeof(int));
				if(params == NULL) {
					//TODO: tbrok
					exit(2);
				}
				
				// tokenize parameters
				token = strtok(pos, ",");
				do {
					params[num_args] = atoi(token);
					num_args++;
				} while(	(token = strtok(NULL, ",")) != NULL && 
							num_args <= filters_installed[i]->num_params_max
						);
						
				if(num_args > filters_installed[i]->num_params_max) {
					free(params);
					return -2;	// too many args!
				}
				
				if(num_args <  filters_installed[i]->num_params_min) {
					free(params);
					return -2; // too few args
				}
			} else {
				params = NULL;
				num_args = 0;
			}
			filter_inst = malloc(sizeof(struct filter_instance));
			
			if(filter_inst == NULL) {
				//TODO: TBROK
				exit(2);
			}
			
			if(setting_verbose > 2)
				fprintf(stderr, "Parsed filter: num_args=%i, filter_type=%s", num_args, filters_installed[i]->name);
				
			// store data
			filter_inst->filter = filters_installed[i];
			filter_inst->params = params;
			filter_inst->num_params = num_args;
			filter_inst->state = NULL;
			// push the filter instance to the filter-chain
			filter_inst->next = filter_head;
			filter_head = filter_inst;
			
			return 0;
		}
		else {
			continue;
		}
	}
	return -1;	// filter not found
}

void free_filters() {
	struct filter_instance* current = filter_head;
	struct filter_instance* next;
	
	while(current != NULL) {
		next = current->next;
		if(current->state != NULL) {
			free(current->state);
		}
		if(current->params != NULL) {
			free(current->params);
		}
		
		free(current);
		
		current = next;
	}
}


void print_filter_help() {
	int i;
	for(i = 0; filters_installed[i] != NULL; i++) {
		printf("**** %-10s \n%s\n\n", filters_installed[i]->name, filters_installed[i]->descr);
	}
}