#include <errno.h>
#include <sys/termios.h>

speed_t cfgetispeed(const struct termios *termios_p) {
  (void)termios_p;
  errno = ENOSYS;
  return 0;
}

speed_t cfgetospeed(const struct termios *termios_p) {
  (void)termios_p;
  errno = ENOSYS;
  return 0;
}

int cfsetispeed(struct termios *termios_p, speed_t speed) {
  (void)termios_p;
  (void)speed;
  errno = ENOSYS;
  return -1;
}

int cfsetospeed(struct termios *termios_p, speed_t speed) {
  (void)termios_p;
  (void)speed;
  errno = ENOSYS;
  return -1;
}

int tcdrain(int fd) {
  (void)fd;
  errno = ENOSYS;
  return -1;
}

int tcflow(int fd, int action) {
  (void)fd;
  (void)action;
  errno = ENOSYS;
  return -1;
}

int tcflush(int fd, int queue_selector) {
  (void)fd;
  (void)queue_selector;
  errno = ENOSYS;
  return -1;
}

int tcgetattr(int fd, struct termios *termios_p) {
  (void)fd;
  (void)termios_p;
  errno = ENOSYS;
  return -1;
}

pid_t tcgetsid(int fd) {
  (void)fd;
  errno = ENOSYS;
  return (pid_t)-1;
}

int tcsendbreak(int fd, int duration) {
  (void)fd;
  (void)duration;
  errno = ENOSYS;
  return -1;
}

int tcsetattr(int fd, int optional_actions, const struct termios *termios_p) {
  (void)fd;
  (void)optional_actions;
  (void)termios_p;
  errno = ENOSYS;
  return -1;
}
