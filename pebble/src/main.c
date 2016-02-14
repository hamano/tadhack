/*
 * main.c
 * motion detection from accelerometer.
 */

#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_output_layer;

#define HIGH_G 1000

#define G_WEAK 1100

#define CMD_ACTION 0
#define CMD_STATE  1

#define STATE_INIT  0
#define STATE_STOP  1
#define STATE_WALK  2
#define STATE_RUN   3
#define STATE_SLEEP 4

const char* state_str[] = {
    "init",
    "stop",
    "walk",
    "run",
    "sleep",
};

static uint8_t stop_count = 0;
static uint8_t walk_count = 0;
static uint8_t run_count = 0;
static uint8_t sleep_count = 0;

static int state = STATE_INIT;

#define ACTION_OK 0
#define ACTION_NG 1

static sqrti(uint32_t x){
    int i = 0;
    int a = 0, c = 0, y = 0, t = x;
    while(t >>= 1) i++;
    for(i += i & 1; i >= 0; i -= 2){
        c = (y << 1 | 1) <= x >> i;
        a = a << 1 | c;
        y = y << 1 | c;
        x -= c * y << i;
        y += c;
    }
    return a;
}

static void send_msg(uint8_t cmd, uint8_t msg){
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    Tuplet value = TupletInteger(cmd, msg);
    dict_write_tuplet(iter, &value);
    app_message_outbox_send();
}

static void change_state(int new_state){
    static char buf[128];
    state = new_state;
    send_msg(CMD_STATE, state);
    APP_LOG(APP_LOG_LEVEL_INFO, "change state: %s", state_str[state]);
    snprintf(buf, sizeof(buf), "state: %s\n", state_str[state]);
    text_layer_set_text(s_output_layer, buf);
}

static void data_handler(AccelData *data, uint32_t num) {
  uint32_t i;
  
  int hx, hy, hz;
  int sx, sy, sz;
  int32_t ax, ay, az;
  int32_t vx, vy, vz;
  int32_t mx, my, mz;

  if(state == STATE_INIT){
      change_state(STATE_STOP);
      return;
  }
  
  sx = hx = data[0].x;
  sy = hy = data[0].y;
  sz = hz = data[0].z;
  for(i=1; i<num;i++){
      if(data[i].did_vibrate){
          APP_LOG(APP_LOG_LEVEL_INFO, "VIBRATE ignore");
          return;
      }
      sx += data[i].x;
      sy += data[i].y;
      sz += data[i].z;
      if(abs(hx) < abs(data[i].x)){
          hx = data[i].x;
      }
      if(abs(hy) < abs(data[i].y)){
          hy = data[i].y;
      }
      if(abs(hz) < abs(data[i].z)){
          hz = data[i].z;
      }
  }

  ax = abs(sx) / num;
  ay = abs(sy) / num;
  az = abs(sz) / num;
  ax *= (sx<0)?-1:1;
  ay *= (sy<0)?-1:1;
  az *= (sz<0)?-1:1;

  mx = hx - ax;
  my = hy - ay;
  mz = hz - az;

  sx = sy = sz = 0;
  for(i=0; i<num;i++){
      int dx, dy, dz;
      dx = data[i].x - ax;
      dy = data[i].y - ay;
      dz = data[i].z - az;
      /*
      APP_LOG(APP_LOG_LEVEL_INFO, "D %5d, %5d, %5d", dx, dy, dz);
      */
      sx += dx * dx;
      sy += dy * dy;
      sz += dz * dz;
  }

  vx = sqrti(sx / num);
  vy = sqrti(sy / num);
  vz = sqrti(sz / num);

  if(mx < -700){
      APP_LOG(APP_LOG_LEVEL_INFO, "push detect %d.", mx);
      APP_LOG(APP_LOG_LEVEL_INFO, "MOT %5d, %5d, %5d", mx, my, mz);
      APP_LOG(APP_LOG_LEVEL_INFO, "AVG  %5d, %5d, %5d", ax, ay, az);
      send_msg(CMD_ACTION, 0);
  }
  /*
  if(mz < -2000){
      APP_LOG(APP_LOG_LEVEL_INFO, "down detect %d.", mz);
      APP_LOG(APP_LOG_LEVEL_INFO, "MOT %5d, %5d, %5d", mx, my, mz);
      APP_LOG(APP_LOG_LEVEL_INFO, "AVG  %5d, %5d, %5d", ax, ay, az);
      send_msg(CMD_ACTION, 1);
  }
  */
  /*
  if(abs(my) > HIGH_G){
      APP_LOG(APP_LOG_LEVEL_INFO, "Y axis %d.", my);
  }
  if(abs(mz) > HIGH_G){
      APP_LOG(APP_LOG_LEVEL_INFO, "Z axis %d.", mz);
  }
  */

    //Show the data
  if (vx > 200 || vy > 200 || vz > 200) {
      //APP_LOG(APP_LOG_LEVEL_INFO, "MOT %5d, %5d, %5d", mx, my, mz);
      //APP_LOG(APP_LOG_LEVEL_INFO, "AVG  %5d, %5d, %5d", ax, ay, az);
      //APP_LOG(APP_LOG_LEVEL_INFO, "STD  %5d, %5d, %5d", vx, vy, vz);
  }

  if(state != STATE_WALK && 1050 < ax && ax < 2000){
      APP_LOG(APP_LOG_LEVEL_INFO, "walk_count  %d", walk_count);
      if(walk_count++ > 3){
          change_state(STATE_WALK);
      }
  }else{
      walk_count = 0;
  }
  if(state != STATE_RUN && vy > 300){
      APP_LOG(APP_LOG_LEVEL_INFO, "run_count  %d", run_count);
      if(run_count++ > 3){
          change_state(STATE_RUN);
      }
  }else{
      run_count = 0;
  }

  if(vx < 200 && vy < 200 && vz < 200){
      //APP_LOG(APP_LOG_LEVEL_INFO, "stop_count  %d", stop_count);
      if(stop_count++ > 10){
          if(state != STATE_STOP){
              change_state(STATE_STOP);
          }
      }
  }else{
      stop_count = 0;
  }

  /*
  if(state == STATE_STOP &&
     vx < 100 && vy < 100 && vz < 100 ){
      APP_LOG(APP_LOG_LEVEL_INFO, "sleep_count  %d", sleep_count);
      if(sleep_count++ > 10){
          if(state != STATE_SLEEP){
              change_state(STATE_SLEEP);
          }
      }
  }else{
      sleep_count = 0;
  }
  */

}

#if 0
int16_t px, py, pz;
int seq=0;

static void data_handler2(AccelData *data, uint32_t num) {
    uint8_t i;
    int16_t dx, dy, dz;

    for(i=0; i<num;i++,seq++){

        if(data[i].did_vibrate){
            APP_LOG(APP_LOG_LEVEL_INFO, "VIBRATE ignore");
            return;
        }
        dx = data[i].x - px;
        dy = data[i].y - py;
        dz = data[i].z - pz;

        px = data[i].x;
        py = data[i].y;
        pz = data[i].z;

        if(abs(dx) < 100 && abs(dy) < 100 && abs(dz) < 100){
            //APP_LOG(APP_LOG_LEVEL_INFO, "%d: skip", i);
            continue;
        }
        APP_LOG(APP_LOG_LEVEL_INFO, "%3d: %d, %d, %d",
                seq, dx, dy, dz);
    }
}
#endif

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  // Create output TextLayer
  s_output_layer = text_layer_create(GRect(5, 0, window_bounds.size.w - 10, window_bounds.size.h));
  text_layer_set_font(s_output_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text(s_output_layer, "No data yet.");
  text_layer_set_overflow_mode(s_output_layer, GTextOverflowModeWordWrap);
  layer_add_child(window_layer, text_layer_get_layer(s_output_layer));
}

static void main_window_unload(Window *window) {
  // Destroy output TextLayer
  text_layer_destroy(s_output_layer);
}

static void recv_handler(DictionaryIterator *iter, void *context)
{

}

static void init() {
  // Create main Window
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);

  app_message_register_inbox_received(recv_handler);
  app_message_open(512, 512);

  // Subscribe to the accelerometer data service
  int num_samples = 10;
  accel_data_service_subscribe(num_samples, data_handler);

  // Choose update rate
  accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);

  change_state(STATE_INIT);
}

static void deinit() {
  // Destroy main Window
  window_destroy(s_main_window);

  accel_data_service_unsubscribe();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
