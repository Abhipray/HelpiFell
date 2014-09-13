#include <pebble.h> //accel.h includes pebble.h

Window *window, *fall_window;

ActionBarLayer *action_bar_fall;
static GBitmap *my_icon_yes;
static GBitmap *my_icon_no;
static TextLayer *s_time_layer;


extern void accel_data_handler(AccelData *data, uint32_t num_samples); //in accel.c
extern void fall_window_load(Window *fall_window);
extern void fall_window_unload(Window *fall_window);
extern void fall_window_appear(Window *fall_window);
extern void fall_window_disappear(Window *fall_window);
extern void fall_no_handler(ClickRecognizerRef recognizer, void *context);
extern void fall_yes_handler(ClickRecognizerRef recognizer, void *context);

static void tick_handler(struct tm *tick_time, TimeUnits units_changed);
void config_provider_fall(void *context);
static void update_time();
static void main_window_unload(Window *window);

// Key values for AppMessage Dictionary
enum {
  STATUS_KEY = 0,
	MESSAGE_KEY = 1
};

// Writes and sends message if not a minor fall
void send_message(int fall){
	DictionaryIterator *iter;
		app_message_outbox_begin(&iter);
	switch(fall) {
	  //send to web, but no text out
		case 0x0: 
			dict_write_uint8(iter, MESSAGE_KEY, 0x0);
			break;
		case 0x1:
		  dict_write_uint8(iter, MESSAGE_KEY, 0x1);
			break;
		default:
		  break;
	}
	dict_write_uint8(iter, STATUS_KEY, 0x1);
	dict_write_end(iter);
  app_message_outbox_send();
}

void launch_fall_window(void){
    accel_data_service_unsubscribe();
    window_stack_push(fall_window, true);
}

static void main_window_load(Window *window) {
  // Create time TextLayer
  s_time_layer = text_layer_create(GRect(0, 55, 144, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "00:00");

  // Improve the layout to be more like a watchface
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  
   // Make sure the time is displayed from the start
  update_time();
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
	}
}
// Called when an incoming message from PebbleKitJS is dropped
static void in_dropped_handler(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");	
}

// Called when PebbleKitJS does not acknowledge receipt of a message
static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}
	
static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

 void config_provider_fall(void *context) {
       window_single_click_subscribe(BUTTON_ID_UP, fall_yes_handler);
       window_single_click_subscribe(BUTTON_ID_DOWN, fall_no_handler);
 }

void init(void) {
	window = window_create();
	// Set handlers to manage the elements inside the Window
  window_set_window_handlers(window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_set_fullscreen(window, true);
  // Show the Window on the watch, with animated=true
  window_stack_push(window, true);
  
  // Create time TextLayer
  s_time_layer = text_layer_create(GRect(0, 55, 144, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "00:00");

  // Improve the layout to be more like a watchface
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  
	
  APP_LOG(APP_LOG_LEVEL_DEBUG, "INIT");
  //Register the accelerometer handle defined in accel.c
  accel_data_service_subscribe(1, accel_data_handler);
  accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);
  
	// Register AppMessage handlers
	app_message_register_inbox_received(in_received_handler); 
	app_message_register_inbox_dropped(in_dropped_handler); 
	app_message_register_outbox_failed(out_failed_handler);
	app_message_register_outbox_sent(outbox_sent_callback);
		
	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
	 
  fall_window = window_create();
  
  window_set_window_handlers(fall_window, (WindowHandlers) {
	  .load = fall_window_load,
    .appear= fall_window_appear,
    .disappear= fall_window_disappear,
    .unload = fall_window_unload,
  });
  //window_set_click_config_provider(fall_window, config_provider_fall);
  
   // Initialize the action bar:
  action_bar_fall = action_bar_layer_create();
  // Set the click config provider:
  action_bar_layer_set_click_config_provider(action_bar_fall,
                                             config_provider_fall);
  // Associate the action bar with the window:
  action_bar_layer_add_to_window(action_bar_fall, fall_window);

  // Set the icons:
  my_icon_yes = gbitmap_create_with_resource(RESOURCE_ID_YES);
  my_icon_no = gbitmap_create_with_resource(RESOURCE_ID_NO);
  action_bar_layer_set_icon(action_bar_fall, BUTTON_ID_UP, my_icon_yes);
  action_bar_layer_set_icon(action_bar_fall, BUTTON_ID_DOWN, my_icon_no);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void main_window_unload(Window *window) {
    // Destroy TextLayer
    text_layer_destroy(s_time_layer);
}

void deinit(void) {
	app_message_deregister_callbacks();
	window_destroy(window);
  window_destroy(fall_window);
  action_bar_layer_destroy(action_bar_fall);
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00";

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    // Use 24 hour format
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    // Use 12 hour format
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

int main( void ) {
	init();
	app_event_loop();
	deinit();
}