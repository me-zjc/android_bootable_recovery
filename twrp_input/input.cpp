#include <fcntl.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <map>

#define TOUCH_DEV "/dev/input/event1"
#define KEY_DEV "/dev/input/event2"

// Android KeyCode → Linux input code 映射
static std::map<int, int> android2linux = {
  //  {3, 102},   // HOME
  { 4, 158 },   // BACK
  { 24, 115 },  // VOLUME_UP
  { 25, 114 },  // VOLUME_DOWN
  { 26, 116 },  // POWER
  { 66, 28 },   // ENTER
  { 62, 57 },   // SPACE
  { 67, 14 },   // DEL
  //  {82, 139},  // MENU
  { 84, 217 },  // SEARCH
};

static void send_event(int fd, __u16 type, __u16 code, __s32 value) {
  struct input_event ev;
  gettimeofday(&ev.time, NULL);
  ev.type = type;
  ev.code = code;
  ev.value = value;
  write(fd, &ev, sizeof(ev));
}

static void do_sync(int fd) {
  send_event(fd, EV_SYN, SYN_REPORT, 0);
}

static void send_key(int android_code) {
  int linux_code = android2linux.count(android_code) ? android2linux[android_code] : android_code;
  int fd = open(KEY_DEV, O_RDWR);
  if (fd < 0) {
    perror("open key device failed");
    return;
  }
  // press
  send_event(fd, EV_KEY, linux_code, 1);
  do_sync(fd);
  // release
  send_event(fd, EV_KEY, linux_code, 0);
  do_sync(fd);
  close(fd);
}

static void send_touch(int x, int y, int down) {
  int fd = open(TOUCH_DEV, O_RDWR);
  if (fd < 0) {
    perror("open touch device failed");
    return;
  }

  if (down) {
    // 选 slot 0
    send_event(fd, EV_ABS, ABS_MT_SLOT, 0);
    // 设置 tracking id
    send_event(fd, EV_ABS, ABS_MT_TRACKING_ID, 0);
    // 设置坐标
    send_event(fd, EV_ABS, ABS_MT_POSITION_X, x);
    send_event(fd, EV_ABS, ABS_MT_POSITION_Y, y);
    // 报告按下
    send_event(fd, EV_KEY, BTN_TOUCH, 1);
  } else {
    // 抬起：清除 tracking id
    send_event(fd, EV_ABS, ABS_MT_SLOT, 0);
    send_event(fd, EV_ABS, ABS_MT_TRACKING_ID, -1);
    // 报告松开
    send_event(fd, EV_KEY, BTN_TOUCH, 0);
  }

  do_sync(fd);
  close(fd);
}

int main(int argc, char** argv) {
  if (argc < 3) {
    printf("Usage:\n");
    printf("  input keyevent <code>\n");
    printf("  input tap <x> <y>\n");
    printf("  input swipe <x1> <y1> <x2> <y2> [steps]\n");
    return 1;
  }

  if (!strcmp(argv[1], "keyevent")) {
    int code = atoi(argv[2]);
    send_key(code);
  } else if (!strcmp(argv[1], "tap")) {
    if (argc < 4) {
      printf("Usage: input tap <x> <y>\n");
      return 1;
    }
    int x = atoi(argv[2]);
    int y = atoi(argv[3]);
    send_touch(x, y, 1);
//    usleep(5 * 1000);
    send_touch(x, y, 0);
  } else if (!strcmp(argv[1], "swipe")) {
    if (argc < 6) {
      printf("Usage: input swipe <x1> <y1> <x2> <y2> [steps]\n");
      return 1;
    }
    int x1 = atoi(argv[2]);
    int y1 = atoi(argv[3]);
    int x2 = atoi(argv[4]);
    int y2 = atoi(argv[5]);
    int steps = (argc > 6) ? atoi(argv[6]) : 10;

    send_touch(x1, y1, 1);
    for (int i = 1; i <= steps; i++) {
      int nx = x1 + (x2 - x1) * i / steps;
      int ny = y1 + (y2 - y1) * i / steps;
      send_touch(nx, ny, 1);
//      usleep(5 * 1000);
    }
    send_touch(x2, y2, 0);
  } else {
    printf("Unknown command: %s\n", argv[1]);
  }
  return 0;
}