#include "weights.h"
#include <pebble.h>

// BEGIN AUTO-GENERATED UI CODE; DO NOT MODIFY
static Window *s_window;
static GBitmap *s_res_yes;
static GBitmap *s_res_no;
static GFont s_res_gothic_24;
static GFont s_res_gothic_24_bold;
static ActionBarLayer *s_actionbarlayer_1;
static TextLayer *s_textlayer_1;
static TextLayer *s_textlayer_2;

static void initialise_ui(void) {
  s_window = window_create();
  window_set_fullscreen(s_window, true);
  
  s_res_yes = gbitmap_create_with_resource(RESOURCE_ID_YES);
  s_res_no = gbitmap_create_with_resource(RESOURCE_ID_NO);
  s_res_gothic_24 = fonts_get_system_font(FONT_KEY_GOTHIC_24);
  s_res_gothic_24_bold = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  // s_actionbarlayer_1
  s_actionbarlayer_1 = action_bar_layer_create();
  action_bar_layer_add_to_window(s_actionbarlayer_1, s_window);
  action_bar_layer_set_background_color(s_actionbarlayer_1, GColorBlack);
  action_bar_layer_set_icon(s_actionbarlayer_1, BUTTON_ID_UP, s_res_yes);
  action_bar_layer_set_icon(s_actionbarlayer_1, BUTTON_ID_DOWN, s_res_no);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_actionbarlayer_1);
  
  // s_textlayer_1
  s_textlayer_1 = text_layer_create(GRect(6, 14, 114, 99));
  text_layer_set_text(s_textlayer_1, "You're close to a weighing machine!");
  text_layer_set_text_alignment(s_textlayer_1, GTextAlignmentCenter);
  text_layer_set_font(s_textlayer_1, s_res_gothic_24);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_1);
  
  // s_textlayer_2
  s_textlayer_2 = text_layer_create(GRect(12, 99, 100, 57));
  text_layer_set_text(s_textlayer_2, "Input your weight?");
  text_layer_set_text_alignment(s_textlayer_2, GTextAlignmentCenter);
  text_layer_set_font(s_textlayer_2, s_res_gothic_24_bold);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_2);
}

static void destroy_ui(void) {
  window_destroy(s_window);
  action_bar_layer_destroy(s_actionbarlayer_1);
  text_layer_destroy(s_textlayer_1);
  text_layer_destroy(s_textlayer_2);
  gbitmap_destroy(s_res_yes);
  gbitmap_destroy(s_res_no);
}
// END AUTO-GENERATED UI CODE

static void handle_window_unload(Window* window) {
  destroy_ui();
}

static void weight_yes_handler(ClickRecognizerRef recognizer, void *context);
static void weight_no_handler(ClickRecognizerRef recognizer, void *context);
static NumberWindow *num_window;
static void number_selected_callback(NumberWindow *win, void *context);
  
extern void send_weight(int w);

static void config_provider_weight(void *context) {
       window_single_click_subscribe(BUTTON_ID_UP, weight_yes_handler);
       window_single_click_subscribe(BUTTON_ID_DOWN, weight_no_handler);
 }

static void number_selected_callback(NumberWindow *win, void *context){
  send_weight(number_window_get_value(win));
  window_stack_pop(true); //remove numwindow
  window_stack_pop(true); //remove weightinput window
}
static void weight_yes_handler(ClickRecognizerRef recognizer, void *context){
  
  num_window = number_window_create("Input Weight", (NumberWindowCallbacks) { .selected = number_selected_callback }, NULL);
  number_window_set_min(num_window, 20);
	number_window_set_max(num_window, 300);
	number_window_set_step_size(num_window, 1);
	number_window_set_value(num_window, 120);
	window_stack_push((Window *)num_window, true);
}

static void weight_no_handler(ClickRecognizerRef recognizer, void *context){
  window_stack_pop(true);
}
void show_weights(void) {
  initialise_ui();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  window_stack_push(s_window, true);
  window_set_click_config_provider(s_window, config_provider_weight);
}

void hide_weights(void) {
  number_window_destroy(num_window);
  window_stack_remove(s_window, true);
}
