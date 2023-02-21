#include <stdio.h>
#include <string.h>

#include <GL/glew.h>
#include <GL/glut.h>

#include "inputs_gl.h"

input_state inputs_1;
input_state inputs_2;
input_state* current_inputs;
input_state* incoming_inputs;

void fill_int_zero (int* location, int length) {
	int i;
	for (i = 0; i < length; i++) {
		location[i] = 0;
	}
}

void copy_ints (int* from, int* to, int count) {
	int i;
	for (i = 0; i < count; i++) {
		to[i] = from[i];
	}
}

input_state* get_inputs () {
	return current_inputs;
}

void init_input_event (input_event* in_event) {
	in_event->event_type = INPUT_EVENT_TYPE_NONE;
	in_event->x = -1;
	in_event->y = -1;
	in_event->arg0 = -1;
	in_event->arg1 = -1;
}

void init_inputs () {
	current_inputs = &inputs_1;
	incoming_inputs = &inputs_2;
	init_input_state (incoming_inputs);
	init_input_state (current_inputs);
}

void reset_key_buffers (input_state* in_state) {
	in_state->num_key_events = 0;
	fill_int_zero ((int*)(&(in_state->keys_pressed)), 256);
	fill_int_zero ((int*)(&(in_state->keys_released)), 256);
}

void reset_mouse_buffers (input_state* in_state) {
	in_state->num_mouse_events = 0;
	fill_int_zero ((int*)(&(in_state->mouse_buttons_clicked)), 4);
	fill_int_zero ((int*)(&(in_state->mouse_buttons_released)), 4);
	in_state->mouse_was_moved = 0;
	in_state->mouse_was_dragged = 0;
}

void init_input_state (input_state* in_state) {
	in_state->num_key_events = 0;
	in_state->num_mouse_events = 0;
	fill_int_zero ((int*)(&(in_state->keys_down)), 256);
	reset_key_buffers (in_state);
	reset_mouse_buffers (in_state);
	in_state->mouse_x = -1;
	in_state->mouse_y = -1;
	in_state->mouse_down = 0;
}

void copy_persistents (input_state* current, input_state* next) {
	copy_ints ((int*)(&(current->keys_down)), (int*)(&(next->keys_down)), 256);
	copy_ints ((int*)(&(current->mouse_buttons_down)), (int*)(&(next->mouse_buttons_down)), 4);
	next->mouse_x = current->mouse_x;
	next->mouse_y = current->mouse_y;
	next->mouse_down = current->mouse_down;
}

void process_inputs (input_state* state) {
	int i;
	
	//Process key inputs
	for (i = 0; i < state->num_key_events; i++) {
		//Get current event
		input_event* ev = &(state->key_events[i]);
		
		//Process the event
		int key = ev->arg0;
		if (ev->event_type == INPUT_EVENT_TYPE_KEY_DOWN) {
			if (!state->keys_down[key]) {
				state->keys_pressed[key] = 1;
			}
			state->keys_down[key] = 1;
		} else if (ev->event_type == INPUT_EVENT_TYPE_KEY_UP) {
			if (state->keys_down[key]) {
				state->keys_released[key] = 1;
			}
			state->keys_down[key] = 0;
		}
	}
	
	//Process mouse inputs
	for (i = 0; i < state->num_mouse_events; i++) {
		
		//Get current event
		input_event* ev = &(state->mouse_events[i]);
		
		//Process the event
		if (ev->event_type == INPUT_EVENT_TYPE_MOUSE_BUTTON) {
			int button = ev->arg0;
			if ((ev->arg1 & 0x1) == 0) {
				if (!state->mouse_buttons_down[button]) {
					state->mouse_buttons_clicked[button] = 1;
				}
				state->mouse_buttons_down[button] = 1;
			} else if ((ev->arg1 & 0x1) == 1) {
				if (state->mouse_buttons_down[button]) {
					state->mouse_buttons_released[button] = 1;
				}
				state->mouse_buttons_down[button] = 0;
			}
		}
		if (ev->event_type == INPUT_EVENT_TYPE_MOUSE_DRAG) {
			state->mouse_was_dragged = 1;
		}
		if (ev->event_type == INPUT_EVENT_TYPE_MOUSE_MOVE) {
			state->mouse_was_moved = 1;
		}
		if (i == state->num_mouse_events - 1) {
			//Mouse coords only need to be updated on the last event
			state->mouse_x = ev->x;
			state->mouse_y = ev->y;
		}
	}
}

void swap_input_buffers () {
	
	//Swap the buffer pointers
	input_state* temp;
	temp = current_inputs;
	current_inputs = incoming_inputs;
	incoming_inputs = temp;
	
	//Process the inputs
	process_inputs (current_inputs);
	
	//Setup the new buffer to recieve inputs
	reset_key_buffers (incoming_inputs);
	reset_mouse_buffers (incoming_inputs);
	copy_persistents (current_inputs, incoming_inputs);
}

void key_handler (unsigned char key, int type, int x, int y) {
	
	//Get current number of key events and overflow check
	int* num_events = &(incoming_inputs->num_key_events);
	if (*num_events > MAX_EVENTS_TRACKED) {
		return; //Sorry, I can't do that
	}
	
	//Get our wonderful event and assign its properties
	input_event* ev = &(incoming_inputs->key_events[*num_events]);
	ev->event_type = type;
	ev->x = x;
	ev->y = y;
	ev->arg0 = (int)key;
	
	//Push the event to the stack
	(*num_events)++;
}

void key_down_handler (unsigned char key, int x, int y) {
	key_handler (key, INPUT_EVENT_TYPE_KEY_DOWN, x, y);
}

void key_up_handler (unsigned char key, int x, int y) {
	key_handler (key, INPUT_EVENT_TYPE_KEY_UP, x, y);
}

void mouse_handler (int button, int state, int x, int y) {
	
	//Get current number of mouse events and overflow check
	int* num_events = &(incoming_inputs->num_mouse_events);
	if (*num_events > MAX_EVENTS_TRACKED) {
		return; //Sorry, I can't do that
	}
	
	//Get our wonderful event and assign its properties
	input_event* ev = &(incoming_inputs->mouse_events[*num_events]);
	ev->event_type = INPUT_EVENT_TYPE_MOUSE_BUTTON;
	ev->x = ((double)x) / glutGet (GLUT_WINDOW_WIDTH);
	ev->y = ((double)y) / glutGet (GLUT_WINDOW_HEIGHT);
	ev->arg0 = button;
	ev->arg1 = state;
	
	//Push the event to the stack
	(*num_events)++;
}

void mouse_motion_handler (int x, int y) {
	//Get current number of mouse events and overflow check
	int* num_events = &(incoming_inputs->num_mouse_events);
	if (*num_events > MAX_EVENTS_TRACKED) {
		return; //Sorry, I can't do that
	}
	
	//Get our wonderful event and assign its properties
	input_event* ev = &(incoming_inputs->mouse_events[*num_events]);
	ev->event_type = INPUT_EVENT_TYPE_MOUSE_DRAG;
	ev->x = ((double)x) / glutGet (GLUT_WINDOW_WIDTH);
	ev->y = ((double)y) / glutGet (GLUT_WINDOW_HEIGHT);
	
	//Push the event to the stack
	(*num_events)++;
}

void passive_mouse_motion_handler (int x, int y) {
	//Get current number of mouse events and overflow check
	int* num_events = &(incoming_inputs->num_mouse_events);
	if (*num_events > MAX_EVENTS_TRACKED) {
		return; //Sorry, I can't do that
	}
	
	//Get our wonderful event and assign its properties
	input_event* ev = &(incoming_inputs->mouse_events[*num_events]);
	ev->event_type = INPUT_EVENT_TYPE_MOUSE_MOVE;
	ev->x = ((double)x) / glutGet (GLUT_WINDOW_WIDTH);
	ev->y = ((double)y) / glutGet (GLUT_WINDOW_HEIGHT);
	
	//Push the event to the stack
	(*num_events)++;
}