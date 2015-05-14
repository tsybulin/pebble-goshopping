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

static Window *window ;
static GBitmap *imgShopped ;
static GBitmap *imgUnshopped ;
static BitmapLayer *markLayer ;
static TextLayer *nameLayer ;
static TextLayer *amountlayer ;
static ActionBarLayer *action_bar ;

static char message[MSG_LEN] ;
static char mark[5] ;
static char name[MSG_LEN] = "Go Shopping!" ;
static char amount[20] ;

static GBitmap *btn_back ;
static GBitmap *btn_forward ;
static GBitmap *btn_mark ;

static void receivedHandler(DictionaryIterator *received, void *context) {
    Tuple *tuple = dict_read_first(received) ;

    while (tuple != NULL) {
        if (tuple->key == CMD_MSG) {
            memset(message, 0, MSG_LEN) ;
            strncpy(message, tuple->value->cstring, MSG_LEN) ;

            int i = 0 ;
            int n = 0 ;
            for (; message[i] != '\0' && message[i] != '\n' && i < MSG_LEN; i++) {
                n++ ;
            }
            memset(mark, 0, 5) ;
            strncpy(mark, &message[0], n) ;

            i++ ;
            n = 0 ;
            int b = i ;
            for (; message[i] != '\0' && message[i] != '\n' && i < MSG_LEN; i++) {
                n++ ;
            }
            
//            memset(no[1], 0, 30) ;
//            strncpy(no[1], &message[b], n) ;
            
            i++ ;
            b = i ;
            n = 0 ;
            for (; message[i] != '\0' && message[i] != '\n' && message[i] != ':' && i < MSG_LEN; i++) {
                n++ ;
            }
            
            memset(name, 0, MSG_LEN) ;
            strncpy(name, &message[b], n) ;
            
            memset(amount, 0, 20) ;
            strcpy(amount, &message[i+2]) ;
            if (strcmp(amount, "1") == 0) {
                strcat(amount, " pc.") ;
            } else {
                strcat(amount, " pcs.") ;
            }
            
            if (strcmp(mark, "💙") == 0) {
                bitmap_layer_set_bitmap(markLayer, imgShopped) ;
            } else {
                bitmap_layer_set_bitmap(markLayer, imgUnshopped) ;
            }
            text_layer_set_text(nameLayer, name) ;
            text_layer_set_text(amountlayer, amount) ;
        
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

    btn_back = gbitmap_create_with_resource(RESOURCE_ID_IMG_BTN_BACK) ;
    btn_forward = gbitmap_create_with_resource(RESOURCE_ID_IMG_BTN_FORWARD) ;
    btn_mark = gbitmap_create_with_resource(RESOURCE_ID_IMG_BTN_MARK) ;
    imgShopped = gbitmap_create_with_resource(RESOURCE_ID_IMG_SHOPPED) ;
    imgUnshopped = gbitmap_create_with_resource(RESOURCE_ID_IMG_UNSHOPPED) ;

    action_bar = action_bar_layer_create() ;
    action_bar_layer_add_to_window(action_bar, window) ;
    action_bar_layer_set_click_config_provider(action_bar, click_config_provider) ;
    action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, btn_back) ;
    action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, btn_forward) ;
    action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, btn_mark) ;
    
    markLayer = bitmap_layer_create((GRect) { .origin = { 5, 0 }, .size = { bounds.size.w - 25, 32} }) ;
    bitmap_layer_set_alignment(markLayer, GAlignCenter) ;
    bitmap_layer_set_bitmap(markLayer, imgUnshopped) ;
    layer_add_child(window_layer, bitmap_layer_get_layer(markLayer)) ;

    nameLayer = text_layer_create((GRect) { .origin = { 5, 32 }, .size = { bounds.size.w - 25, 88} }) ;
    text_layer_set_text(nameLayer, name) ;
    text_layer_set_text_alignment(nameLayer, GTextAlignmentLeft) ;
    text_layer_set_overflow_mode(nameLayer, GTextOverflowModeWordWrap) ;
    text_layer_set_font(nameLayer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD)) ;
    layer_add_child(window_layer, text_layer_get_layer(nameLayer)) ;

    amountlayer = text_layer_create((GRect) { .origin = { 5, 120 }, .size = { bounds.size.w - 25, 32} }) ;
    text_layer_set_text_alignment(amountlayer, GTextAlignmentLeft) ;
    text_layer_set_overflow_mode(amountlayer, GTextOverflowModeWordWrap) ;
    text_layer_set_font(amountlayer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD)) ;
    layer_add_child(window_layer, text_layer_get_layer(amountlayer)) ;
}

static void window_unload(Window *window) {
    action_bar_layer_destroy(action_bar) ;
    gbitmap_destroy(btn_back) ;
    gbitmap_destroy(btn_forward) ;
    gbitmap_destroy(btn_mark) ;
    text_layer_destroy(amountlayer) ;
    text_layer_destroy(nameLayer) ;
    bitmap_layer_destroy(markLayer) ;
    gbitmap_destroy(imgShopped) ;
    gbitmap_destroy(imgUnshopped) ;
}

static void init(void) {
    window = window_create() ;
    window_set_window_handlers(window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    }) ;

    window_stack_push(window, true) ;

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
