#include <kazusa/connection.h>
#include <kazusa/config.h>
#include <kazusa/stomp.h>
#include <kazusa/signal.h>
#include <kazusa/common.h>

#include <pthread.h>

static int cleanup_connection(void *data) {
  int *sock = (int *)data;
  int ret = RET_ERROR;

  if(sock == NULL) {
    close(*sock);

    ret = RET_SUCCESS;
  }

  return ret;
}

static void *connection_co_worker(void *data) {
  int *sock = (int *)data;
  char buf[BUFSIZE];
  void *driver_cache = NULL;
  sighandle_t *handler;

  if(sock != NULL) {
    /* initialize processing after established connection */
    handler = set_signal_handler(cleanup_connection, sock);
  
    while(1) {
      memset(buf, 0, BUFSIZE);
      if(! read(*sock, buf, sizeof(buf))) {
        break;
      }
      stomp_recv_data(buf, strlen(buf), *sock, &driver_cache);
    }

    del_signal_handler(handler);

    close(*sock);
  }

  return NULL;
}

void *connection_worker(void *data) {
  kd_config *conf = (kd_config *)data;
  struct sockaddr_in addr;
  int sd;
  int acc_sd;
 
  if((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket");
    return NULL;
  }
 
  addr.sin_family = AF_INET;
  addr.sin_port = htons(conf->port);
  addr.sin_addr.s_addr = INADDR_ANY;
 
  if(bind(sd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("bind");
    return NULL;
  }
 
  if(listen(sd, QUEUENUM) < 0) {
    perror("listen");
    return NULL;
  }

  set_signal_handler(cleanup_connection, &sd);
 
  struct sockaddr_in from_addr;
  socklen_t sin_size = sizeof(struct sockaddr_in);
  pthread_t thread_id;
  while(1) {
    if((acc_sd = accept(sd, (struct sockaddr *)&from_addr, &sin_size)) < 0) {
      perror("accept");
      return NULL;
    }

    pthread_create(&thread_id, NULL, &connection_co_worker, &acc_sd);
  }
 
  return NULL;
}

int send_msg(int sock, char **msg) {
  int i;
  char *line;

  for(i=0; (line = msg[i])!=NULL; i++) {
    if(send(sock, line, strlen(line), 0) < 0) {
      printf("[ERROR] failed to send message [%s]\n", line);
      return RET_ERROR;
    }
  }
  send(sock, "\0", 1, 0);

  return RET_SUCCESS;
}