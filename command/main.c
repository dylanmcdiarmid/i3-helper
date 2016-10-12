/* 
 * The majority of this is cribbed from i3-msg. I don't really know how to write C, so use at your own risk.
 */
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <err.h>
#include <errno.h>
#include <stdint.h>
#include <limits.h>
#include <stdarg.h>

int sasprintf(char **strp, const char *fmt, ...) {
  va_list args;
  int result;

  va_start(args, fmt);
  if ((result = vasprintf(strp, fmt, args)) == -1)
      err(EXIT_FAILURE, "asprintf(%s)", fmt);
  va_end(args);
  return result;
}

char *sstrdup(const char *str) {
  char *result = strdup(str);
  if (result == NULL)
      err(EXIT_FAILURE, "strdup()");
  return result;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("You must provide a command to send to the i3-helper daemon.\n");
    return 0;
  }
  char *socket_path = "/tmp/i3helper.sock";
  char *payload = NULL;
  int i = 1;
  for (i = 1; i < argc; i++) {
    if (!payload) {
        payload = sstrdup(argv[i]);
    } else {
        char *both;
        sasprintf(&both, "%s %s", payload, argv[i]);
        free(payload);
        payload = both;
    }
  }
  int sockfd = socket(AF_LOCAL, SOCK_STREAM, 0);
  if (sockfd == -1)
    err(EXIT_FAILURE, "Could not create socket");

  struct sockaddr_un addr;
  memset(&addr, 0, sizeof(struct sockaddr_un));
  addr.sun_family = AF_LOCAL;
  strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);
  if (connect(sockfd, (const struct sockaddr *)&addr, sizeof(struct sockaddr_un)) < 0)
    err(EXIT_FAILURE, "Could not connect to i3-helper socket");
  size_t payload_size = strlen(payload);
  size_t written = 0;
  size_t n = 0;
  while (written < payload_size) {
    n = write(sockfd, payload, payload_size - written);
    if (n == -1) {
      if (errno == EINTR || errno == EAGAIN)
        continue;
      return n;
    }
    written += n;
  }
  return 0;
}
