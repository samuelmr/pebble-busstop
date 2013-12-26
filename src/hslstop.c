#include <pebble.h>
int number_of_deps = 0;
int max_deps = 7;
int padding = 24;
const bool animated = true;
TextLayer *stop_name;
TextLayer *init_text;
Window *window;
Layer *window_layer;
GRect bounds;

typedef struct{
  char line[7];
  char dest[40];
  char time[5];
  TextLayer *line_text;
  TextLayer *dest_text;
  TextLayer *time_text;
} Departure;

static Departure departures[7];

/*
static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
}

static void back_click_handler(ClickRecognizerRef recognizer, void *context) {
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  // window_stack_pop(animated);
}
*/

/*
static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}
*/

static void window_load(Window *window) {
}

static void window_unload(Window *window) {
  for (int i = number_of_deps - 1; i >= 0; i--) {
    Departure *dep = &departures[i];
    if (dep && dep->time_text) {
      text_layer_destroy(dep->time_text);
    }
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Departure %d/%d, removed time_text", i, number_of_deps);
    if (dep && dep->dest_text) {
      text_layer_destroy(dep->dest_text);
    }
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Departure %d/%d, removed dest_text", i, number_of_deps);
 /* will crash the app
    if (dep && dep->line_text) {
      text_layer_destroy(dep->line_text);
    }
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Departure %d/%d, removed line_text", i, number_of_deps);
 */
  }
  if (stop_name) {
   text_layer_destroy(stop_name);
  }
  APP_LOG(APP_LOG_LEVEL_DEBUG, "removed stop_name");
}

void in_received_handler(DictionaryIterator *received, void *context) {
  if (number_of_deps >= max_deps) {
    return;
  }
  Tuple *part;
	    
  Departure *dep = &departures[number_of_deps];

  part = dict_read_first(received);
  if (number_of_deps == 0) {
    if (init_text) {
      text_layer_destroy(init_text);
    }
    stop_name = text_layer_create(GRect(0, 0, bounds.size.w, 23));
    text_layer_set_font(stop_name, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text_alignment(stop_name, GTextAlignmentLeft);
    layer_add_child(window_layer, text_layer_get_layer(stop_name));
    text_layer_set_text(stop_name, part->value->cstring);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Set stop name to %s", part->value->cstring);
  }
  
  part = dict_read_next(received);
  strcpy(dep->line, part->value->cstring);
  dep->line_text = text_layer_create(GRect(0, padding, 30, 20));
  text_layer_set_background_color(dep->line_text, GColorBlack);
  text_layer_set_text_color(dep->line_text, GColorWhite);
  text_layer_set_font(dep->line_text, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(dep->line_text, GTextAlignmentCenter);
  text_layer_set_text(dep->line_text, dep->line);
  layer_add_child(window_layer, text_layer_get_layer(dep->line_text));

  part = dict_read_next(received);
  strcpy(dep->dest, part->value->cstring);
  dep->dest_text = text_layer_create(GRect(31, padding+4, 72, 16));
  text_layer_set_font(dep->dest_text, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_overflow_mode(dep->dest_text, GTextOverflowModeTrailingEllipsis);	
  text_layer_set_text(dep->dest_text, dep->dest);
  layer_add_child(window_layer, text_layer_get_layer(dep->dest_text));

  part = dict_read_next(received);
  strcpy(dep->time, part->value->cstring);
  dep->time_text = text_layer_create(GRect(104, padding, 40, 20));
  text_layer_set_background_color(dep->time_text, GColorBlack);
  text_layer_set_text_color(dep->time_text, GColorWhite);
  text_layer_set_font(dep->time_text, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(dep->time_text, GTextAlignmentRight);
  text_layer_set_text(dep->time_text, dep->time);
  layer_add_child(window_layer, text_layer_get_layer(dep->time_text));

  padding += 18;
  number_of_deps++;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "%d deps, %s to %s at %s", number_of_deps, dep->line, dep->dest, dep->time);
}

void in_dropped_handler(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Message from phone dropped: %d", reason);
  if (init_text) {
    text_layer_set_text(init_text, "Message from phone dropped...");
  }
}

static void init(void) {
  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
  number_of_deps = 0;
  const uint32_t inbound_size = 1024;
  const uint32_t outbound_size = 1024;
  app_message_open(inbound_size, outbound_size);
  window = window_create();
/*
  window_set_click_config_provider(window, click_config_provider);
*/
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_layer = window_get_root_layer(window);
  bounds = layer_get_frame(window_layer);
  window_stack_push(window, animated);
  init_text = text_layer_create(GRect(5, 50, bounds.size.w-10, 50));
  text_layer_set_font(init_text, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(init_text, GTextAlignmentLeft);
  layer_add_child(window_layer, text_layer_get_layer(init_text));
  text_layer_set_text(init_text, "Loading data...");
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
