#include <pebble.h>

static Window *main_window;

static Layer *icon_layer;

void update_icon_layer(Layer *layer, GContext* ctx){
  graphics_context_set_fill_color(ctx, GColorClear);
  GPoint p = {.x = 50, .y = 50};
  graphics_fill_circle(ctx, p, 50);  
}

static void setup_icon_layer(Layer *window_layer, GRect bounds) {
  icon_layer = layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));
  
  layer_set_update_proc(icon_layer, update_icon_layer);
  layer_add_child(window_layer, icon_layer);
}


static void setup_main_window(Window *window) {
  window_set_background_color(window, GColorBlack);
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  setup_icon_layer(window_layer, bounds);
}

static void tear_down_main_window(Window *window) {
  layer_destroy(icon_layer);
}

void send_int(int value){
  DictionaryIterator *out_iter;
  
  AppMessageResult result = app_message_outbox_begin(&out_iter);
  if(result == APP_MSG_OK) {
    dict_write_int(out_iter, MESSAGE_KEY_PAGE_TURN, &value, sizeof(int), true);
  
    result = app_message_outbox_send();
    if(result != APP_MSG_OK) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending the outbox: %d", (int)result);
    }
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error preparing the outbox: %d", (int)result);
  }
}

void on_up_button_clicked(ClickRecognizerRef recognizer, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "up");
  send_int(0);
}

void on_down_button_clicked(ClickRecognizerRef recognizer, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "down");
  send_int(1);
}

void button_click_config_provider(Window *window) {
  window_single_click_subscribe(BUTTON_ID_UP, on_up_button_clicked);
  window_single_click_subscribe(BUTTON_ID_DOWN, on_down_button_clicked);  
}

void setup(){
  main_window = window_create();
  
  window_set_window_handlers(main_window, (WindowHandlers) {
    .load = setup_main_window,
    .unload = tear_down_main_window
  });
  
  window_stack_push(main_window, true);
  
  app_message_open(APP_MESSAGE_INBOX_SIZE_MINIMUM, APP_MESSAGE_OUTBOX_SIZE_MINIMUM);
  window_set_click_config_provider(main_window, (ClickConfigProvider) button_click_config_provider);
  
}

void tear_down(){
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Tear down");
  window_destroy(main_window);
}

int main() {
  setup();
  app_event_loop();
  tear_down();
}