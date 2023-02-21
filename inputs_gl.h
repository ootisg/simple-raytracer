#ifndef INPUTS_H
#define INPUTS_H

#define INPUT_EVENT_TYPE_NONE 0
#define INPUT_EVENT_TYPE_KEY_UP 1
#define INPUT_EVENT_TYPE_KEY_DOWN 2
#define INPUT_EVENT_TYPE_MOUSE_BUTTON 3
#define INPUT_EVENT_TYPE_MOUSE_DRAG 4
#define INPUT_EVENT_TYPE_MOUSE_MOVE 5

#define MAX_EVENTS_TRACKED 1024

struct input_event {
	int event_type;
	double x;
	double y;
	int arg0;
	int arg1;
};

#define input_event struct input_event

struct input_state {
	int num_key_events;
	int num_mouse_events;
	input_event key_events[MAX_EVENTS_TRACKED];
	input_event mouse_events[MAX_EVENTS_TRACKED];
	int keys_down[256];
	int keys_pressed[256];
	int keys_released[256];
	int mouse_buttons_down[4];
	int mouse_buttons_clicked[4];
	int mouse_buttons_released[4];
	double mouse_x;
	double mouse_y;
	int mouse_down;
	int mouse_was_moved;
	int mouse_was_dragged;
};

#define input_state struct input_state

void init_inputs ();
input_state* get_inputs ();

void init_input_event (input_event* in_event);

void init_input_state (input_state* in_state);
void reset_key_buffers (input_state* in_state);
void reset_mouse_buffers (input_state* in_state);

void key_up_handler (unsigned char key, int x, int y);
void key_down_handler (unsigned char key, int x, int y);
void mouse_handler (int button, int state, int x, int y);
void mouse_motion_handler (int x, int y);
void passive_mouse_motion_handler (int x, int y);

void copy_persistents (input_state* current, input_state* next);
void process_inputs (input_state* state);

void swap_input_buffers ();

#endif