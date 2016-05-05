#include <newt/list.h>
#include <newt/stomp.h>
#include <newt/common.h>
#include <newt/logger.h>

#include <assert.h>

#define QNAME_LENGTH (256)

struct dstinfo_t {
  char *qname;
  char *tid;
};

static int transaction_callback(frame_t *frame) {
  int ret = RET_ERROR;

  if(frame != NULL && frame->transaction_data != NULL) {
    enqueue((void *)frame, (char *)frame->transaction_data);

    ret = RET_SUCCESS;
  }

  return ret;
}

static int handler_transaction(char *context, void *data) {
  struct dstinfo_t *dstinfo = (struct dstinfo_t *)data;
  int ret = RET_ERROR;

  if(dstinfo != NULL) {
    dstinfo->tid = context;

    ret = RET_SUCCESS;
  }

  return ret;
}

static int handler_destination(char *context, void *data) {
  struct dstinfo_t *dstinfo = (struct dstinfo_t *)data;
  int ret = RET_ERROR;

  if(dstinfo != NULL) {
    dstinfo->qname = context;

    ret = RET_SUCCESS;
  }

  return ret;
}

static stomp_header_handler_t handlers[] = {
  {"destination:", handler_destination},
  {"transaction:", handler_transaction},
  {0},
};

frame_t *handler_stomp_send(frame_t *frame) {
  struct dstinfo_t dstinfo = {0};

  assert(frame != NULL);
  assert(frame->cinfo != NULL);

  if(iterate_header(&frame->h_attrs, handlers, &dstinfo) == RET_ERROR) {
    err("(handle_stomp_send) validation error");
    stomp_send_error(frame->sock, "failed to validate header\n");
    return NULL;
  }

  if(dstinfo.qname == NULL) {
    stomp_send_error(frame->sock, "no destination is specified\n");
    return NULL;
  }

  if(dstinfo.tid == NULL) {
    enqueue((void *)frame, dstinfo.qname);
  } else {
    frame->transaction_data = (void *)dstinfo.qname;
    transaction_add(dstinfo.tid, frame);
  }

  return frame;
}
