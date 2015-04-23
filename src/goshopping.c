#include <pebble.h>

#define MSG_LEN 120

enum {
    CMD_MSG       =   1,
    CMD_MARK      =   2,
    CMD_PREV      =   3,
    CMD_NEXT      =   4,
    CMD_FIRST     =   5,
    CMD_BYE       =   6
} ;

static Window *window;
static TextLayer *text_layer;
static char message[MSG_LEN] = "Go Shopping!" ;
static GBitmap *buttonsImage ;
static BitmapLayer *buttonsLayer ;

static void receivedHandler(DictionaryIterator *received, void *context) {
    Tuple *tuple = dict_read_first(received) ;

    while (tuple != NULL) {
        if (tuple->key == CMD_MSG) {
            strncpy(message, tuple->value->cstring, MSG_LEN) ;
            text_layer_set_text(text_layer, message) ;
        
            app_comm_set_sniff_interval(SNIFF_INTERVAL_NORMAL) ;
        }
        
        tuple = dict_read_next(received) ;
    }
}

static void send(int key, int msg) {
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    dict_write_int(iter, key, &msg, sizeof(int), true) ;
    app_message_outbox_send() ;
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
//    APP_LOG(APP_LOG_LEVEL_DEBUG, "CMD_MARK") ;
    send(CMD_MARK, 1) ;
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
//    APP_LOG(APP_LOG_LEVEL_DEBUG, "CMD_PREV") ;
    send(CMD_PREV, 1) ;
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
//    APP_LOG(APP_LOG_LEVEL_DEBUG, "CMD_NEXT") ;
    send(CMD_NEXT, 1) ;
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window) ;
    GRect bounds = layer_get_bounds(window_layer) ;

    buttonsImage = gbitmap_create_with_resource(RESOURCE_ID_IMG_BUTTONS_WHITE) ;
    buttonsLayer = bitmap_layer_create((GRect) { .origin = { bounds.size.w - 20, 0 }, .size = { 20, bounds.size.h} }) ;
    bitmap_layer_set_bitmap(buttonsLayer, buttonsImage) ;

    layer_add_child(window_layer, bitmap_layer_get_layer(buttonsLayer)) ;

    text_layer = text_layer_create((GRect) { .origin = { 0, 0 }, .size = { bounds.size.w - 20, bounds.size.h} }) ;
    text_layer_set_text(text_layer, message) ;
    text_layer_set_text_alignment(text_layer, GTextAlignmentCenter) ;
    text_layer_set_overflow_mode(text_layer, GTextOverflowModeWordWrap) ;
    text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD)) ;

    layer_add_child(window_layer, text_layer_get_layer(text_layer)) ;
}

static void window_unload(Window *window) {
    layer_remove_from_parent(bitmap_layer_get_layer(buttonsLayer)) ;
    bitmap_layer_destroy(buttonsLayer) ;
    gbitmap_destroy(buttonsImage) ;
    text_layer_destroy(text_layer);
}

static void init(void) {
    window = window_create() ;
    window_set_click_config_provider(window, click_config_provider) ;
    window_set_window_handlers(window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    }) ;

    window_stack_push(window, true) ;
    window_set_background_color(window, GColorBlack) ;

    app_message_register_inbox_received(receivedHandler) ;
    app_message_open(MSG_LEN, MSG_LEN) ;

    send(CMD_FIRST, 1) ;
}

static void deinit(void) {
    send(CMD_BYE, 1) ;
    app_message_deregister_callbacks() ;
    window_destroy(window) ;
}

int main(void) {
  init();

//  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
