#include <pebble.h> //accel.h includes pebble.h
#define BUF_SIZE 50
  
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
extern void show_weights(void);

void tick_handler(struct tm *tick_time, TimeUnits units_changed);
void config_provider_fall(void *context);
static void update_time();
static void main_window_unload(Window *window);
static void config_provider_main(void *context);
void send_weight(int w);

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

// Writes and sends message if not a minor fall
void send_weight(int w){
  APP_LOG(APP_LOG_LEVEL_DEBUG, "send_weight");
	DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  dict_write_uint8(iter, MESSAGE_KEY, w);
	dict_write_uint8(iter, STATUS_KEY, 0x2);
	dict_write_end(iter);
  app_message_outbox_send();
}

void launch_fall_window(void){
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Launch fall window");
    accel_data_service_unsubscribe();
    window_stack_push(fall_window, true);
}

static void main_window_load(Window *window) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "In main_window_load");
  // Create time TextLayer
  s_time_layer = text_layer_create(GRect(0, 55, 144, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  //text_layer_set_text(s_time_layer, "00:00");

  // Improve the layout to be more like a watchface
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
 
  // Make sure the time is displayed from the start
  APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "IN MAIN WINDOW LOAD, About to update_time");
  update_time();    
}

static void main_window_appear(Window *window){
  //Register the accelerometer handle defined in accel.c
  APP_LOG(APP_LOG_LEVEL_DEBUG, "main_window_appear");
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  accel_data_service_subscribe(BUF_SIZE, accel_data_handler);
  accel_service_set_sampling_rate(ACCEL_SAMPLING_50HZ);
}

static void main_window_disappear(Window *window){
  //Register the accelerometer handle defined in accel.c
  accel_data_service_unsubscribe();
  tick_timer_service_unsubscribe();
}


// Called when a message is received from PebbleKitJS
static void in_received_handler(DictionaryIterator *received, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "In recieved_handler");
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

static void panic_call(ClickRecognizerRef recognizer, void *context){
    launch_fall_window();
}

static void weights_handle(ClickRecognizerRef recognizer, void *context){
    show_weights();
}
static void config_provider_main(void *context) {
    window_long_click_subscribe(BUTTON_ID_SELECT, 1200, panic_call, NULL);
    window_long_click_subscribe(BUTTON_ID_UP, 200, weights_handle, NULL);
 }

void init(void) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "INIT started");
	window = window_create();
	// Set handlers to manage the elements inside the Window
  window_set_window_handlers(window, (WindowHandlers) {
    .load = main_window_load,
    .appear= main_window_appear,
    .disappear= main_window_disappear,
    .unload = main_window_unload
  });
  window_set_fullscreen(window, true);
  // Show the Window on the watch, with animated=true
  window_stack_push(window, true);
      
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
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
  
  window_set_click_config_provider(window, config_provider_main);
  
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
  APP_LOG(APP_LOG_LEVEL_DEBUG, "INIT FINISHED");
}

static void main_window_unload(Window *window) {
    accel_data_service_unsubscribe();
    // Destroy TextLayer
    text_layer_destroy(s_time_layer);
}

void deinit(void) {
  window_destroy(fall_window);
	window_destroy(window);
  action_bar_layer_destroy(action_bar_fall);
  app_message_deregister_callbacks();
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00";

  // Write the current hours and minutes into the buffer
  if(tick_time != NULL)
  {
      if(clock_is_24h_style() == true) {
      // Use 24 hour format
      strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
    } else {
      // Use 12 hour format
      strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
    }
  }
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
}

void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

int main( void ) {
	init();
	app_event_loop();
	deinit();
}