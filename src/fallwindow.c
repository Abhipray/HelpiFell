#include<pebble.h>
#define COUNTDOWN 10
  
static TextLayer *text_layer, *note_layer, *note_layer2;
static int countdown = COUNTDOWN;

extern void send_message(int fall_no);
extern void accel_data_handler(AccelData *data, uint32_t num_samples);

void fall_yes_handler(ClickRecognizerRef recognizer, void *context);
void fall_no_handler(ClickRecognizerRef recognizer, void *context);
void fall_window_load(Window *fall_window);
void fall_window_unload(Window *fall_window);
void fall_window_appear(Window *fall_window);
void fall_window_disappear(Window *fall_window);
  
static void countdown_handler(struct tm *tick_time, TimeUnits units_changed);

void fall_window_load(Window *window2) {
  Layer *window_layer = window_get_root_layer(window2);
  GRect bounds = layer_get_bounds(window_layer);
  //text_layer = text_layer_create((GRect) { .origin = { 0, 72 }});
  text_layer = text_layer_create((GRect) { { 5, 0 }, { bounds.size.w - 2*5, bounds.size.h } });
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
  text_layer_set_text(text_layer, "10");
  
  note_layer = text_layer_create((GRect) { .origin = { 0, 72 }, .size = { 0.7 * bounds.size.w, 28 } });
  text_layer_set_font(note_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  text_layer_set_text(note_layer, "Did you fall?");
  text_layer_set_text_alignment(note_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(note_layer));
  
  note_layer2 = text_layer_create((GRect) { .origin = { 0, 100 }, .size = { 0.7*bounds.size.w, 28 } });
  text_layer_set_font(note_layer2, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  text_layer_set_text(note_layer2, "Are you OK?");
  text_layer_set_text_alignment(note_layer2, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(note_layer2));
  
}

void fall_window_unload(Window *fall_window) {
  text_layer_destroy(text_layer);
  text_layer_destroy(note_layer);
  text_layer_destroy(note_layer2);
}

void fall_window_appear(Window *fall_window){
  vibes_long_pulse();
  countdown = COUNTDOWN;
  tick_timer_service_subscribe(SECOND_UNIT, countdown_handler);
}

static void countdown_handler(struct tm *tick_time, TimeUnits units_changed){
  APP_LOG(APP_LOG_LEVEL_DEBUG, "countdown %d", countdown);
  if(countdown >= 0)
  {
    static char buf[] = "10";
    snprintf(buf, sizeof(buf), "%d", countdown);
    text_layer_set_text(text_layer, buf);
    
    if(countdown < 3){
      vibes_short_pulse();
    }
  }
  
  if(countdown == -1){
    send_message(1);
    text_layer_set_text(note_layer, "Call for help");
    text_layer_set_text(note_layer2, "sent");
  }
  
  else if (countdown < -5)
  {
    
    window_stack_pop(true);
  }
  countdown--;
}

void fall_window_disappear(Window *fall_window){
  tick_timer_service_unsubscribe();
  //Register the accelerometer handle defined in accel.c
  accel_data_service_subscribe(1, accel_data_handler);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "fall_window_disappear");
  accel_service_set_sampling_rate(ACCEL_SAMPLING_50HZ);
}

void fall_yes_handler(ClickRecognizerRef recognizer, void *context){
  APP_LOG(APP_LOG_LEVEL_DEBUG, "fall_yes_handler");
  send_message(0);
  window_stack_pop(true);
}

void fall_no_handler(ClickRecognizerRef recognizer, void *context){
    APP_LOG(APP_LOG_LEVEL_DEBUG, "fall_no_handler");
    send_message(1);
    text_layer_set_text(note_layer, "Call for help");
    text_layer_set_text(note_layer2, "sent");
    countdown = -2;
}