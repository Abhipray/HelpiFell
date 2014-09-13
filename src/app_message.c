#include <pebble.h> //accel.h includes pebble.h
#define FALL_KEY;
Window *window, *fall_window;

static ActionBarLayer *action_bar_fall;
static GBitmap *my_action_yes;
static GBitmap *my_action_no;

extern void accel_data_handler(AccelData *data, uint32_t num_samples); //in accel.c
extern void fall_window_load(Window *fall_window);
extern void fall_window_unload(Window *fall_window);
extern void fall_window_appear(Window *fall_window);
extern void fall_window_disappear(Window *fall_window);
extern void fall_no_handler(ClickRecognizerRef recognizer, void *context);
extern void fall_yes_handler(ClickRecognizerRef recognizer, void *context);

void config_provider_fall(void *context);

// Key values for AppMessage Dictionary
enum {
  STATUS_KEY = 0;
	MESSAGE_KEY = 1;
};

// Writes and sends message if not a minor fall
void send_message(int fall){
	DictionaryIterator *iter;
		app_message_outbox_begin(&iter);
	switch(fall) {
	  //send to web, but no text out
		case 0x0: 
			dict_write_uint8(iter, FALL_KEY, 0x0);
			break;
		case 0x1:
		  dict_write_uint8(iter, FALL_KEY, 0x1);
			break;
		default:
		  break;
	}
	F
	dict_write_end(iter);
  	app_message_outbox_send();
}

void launch_fall_window(void){
    accel_data_service_unsubscribe();
    window_stack_push(fall_window, true/*animated*/);
}

// Called when a message is received from PebbleKitJS
static void in_received_handler(DictionaryIterator *received, void *context) {
	Tuple *tuple;
	
	tuple = dict_find(received, STATUS_KEY);
	if(tuple) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Received Status: %d", (int)tuple->value->uint32); 
	}
	
	tuple = dict_find(received, MESSAGE_KEY);
	if(tuple) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Received Message: %s", tuple->value->cstring);
	}}
}
// Called when an incoming message from PebbleKitJS is dropped
static void in_dropped_handler(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");	
}

// Called when PebbleKitJS does not acknowledge receipt of a message
static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}
	
static void out_sent_handler(DictionaryIterator *iter, AppMessageResuly reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

 void config_provider_fall(void *context) {
       window_single_click_subscribe(BUTTON_ID_UP, fall_yes_handler);
       window_single_click_subscribe(BUTTON_ID_DOWN, fall_no_handler);
 }

void init(void) {
	window = window_create();
	window_stack_push(window, true);
	
  APP_LOG(APP_LOG_LEVEL_DEBUG, "INIT");
  //Register the accelerometer handle defined in accel.c
  accel_data_service_subscribe(1, accel_data_handler);
  accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);
  
	// Register AppMessage handlers
	app_message_register_inbox_received(in_received_handler); 
	app_message_register_inbox_dropped(in_dropped_handler); 
	app_message_register_outbox_failed(out_failed_handler);
	app_message_register_outbox_sent(out_sent_handler);
		
	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
	 
  fall_window = window_create();
  //window_set_click_config_provider(fall_window, click_config_provider2);
  window_set_window_handlers(fall_window, (WindowHandlers) {
	  .load = fall_window_load,
    .appear= fall_window_appear,
    .disappear= fall_window_disappear,
    .unload = fall_window_unload,
  });
  window_set_click_config_provider(fall_window, config_provider_fall);
  
   // Initialize the action bar:
  action_bar_fall = action_bar_layer_create();
  // Associate the action bar with the window:
  action_bar_layer_add_to_window(action_bar_fall, fall_window);
  // Set the click config provider:
  action_bar_layer_set_click_config_provider(action_bar_fall,
                                             click_config_provider);
  // Set the icons:
  my_icon_yes = gbitmap_create_with_resource(RESOURCE_ID_PLUS);
  my_icon_no = gbitmap_create_with_resource(RESOURCE_ID_MINUS);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, &my_icon_yes);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, &my_icon_no);
}

void deinit(void) {
	app_message_deregister_callbacks();
	window_destroy(window);
  window_destroy(fall_window);
}

int main( void ) {
	init();
	app_event_loop();
	deinit();
}