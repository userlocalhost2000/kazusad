#include <newt/common.h>
#include <newt/stomp_management_worker.h>
#include <newt/frame.h>

#include <assert.h>

struct attrinfo_t {
  char *id;
};

static int handler_id(char *context, void *data, linedata_t *_hdr) {
  struct attrinfo_t *attrinfo = (struct attrinfo_t *)data;
  int ret = RET_ERROR;

  if(attrinfo != NULL) {
    attrinfo->id = context;

    ret = RET_SUCCESS;
  }

  return ret;
}

static stomp_header_handler_t handlers[] = {
  {"id:", handler_id},
  {0},
};

frame_t *handler_stomp_ack(frame_t *frame) {
  struct attrinfo_t attrinfo = {0};

  assert(frame != NULL);
  assert(frame->cinfo != NULL);

  if(iterate_header(&frame->h_attrs, handlers, &attrinfo) == RET_ERROR) {
    err("(handle_stomp_ack) validation error");
    stomp_send_error(frame->sock, "failed to validate header\n");
    return NULL;
  }

  /* implementation for ack */

  return NULL;
}
