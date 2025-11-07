#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H
#include "um_fp.h"
// basic state machine
typedef void *(*stateFunction_voidFn)(um_fp);
typedef struct {
  stateFunction_voidFn sf;
  um_fp caller;
} stateNext;
typedef stateNext (*stateFunction)(um_fp caller);
#define nullState ((stateFunction_voidFn)0)
#define nullNext ((stateNext){.sf = nullState, .caller = nullUmf})
void StateMachine_loop(stateFunction entryPoint);
#ifdef __cplusplus
thread_local
#else
_Thread_local
#endif
    static uint8_t stateFnNameReturnBuffer[1024];
// since they call eachother declarations are required
#define SFDec(fnName) stateNext fnName(um_fp)
#define SFDef(fnName, ...) stateNext fnName(um_fp caller) __VA_ARGS__
// copy pointer into  buffer and then return that
#define SFReturn(fnName)                                                       \
  return ({                                                                    \
    memcpy(stateFnNameReturnBuffer, __FUNCTION__,                              \
           sizeof(__FUNCTION__) / sizeof(uint8_t));                            \
    ((stateNext){                                                              \
        .sf = (stateFunction_voidFn)fnName,                                    \
        .caller = ((um_fp){.ptr = stateFnNameReturnBuffer,                     \
                           .width = (sizeof(__FUNCTION__) / sizeof(uint8_t)) - \
                                    1})});                                     \
  })
// SFReturn(0) will end a state machine
#endif //  STATE_MACHINE_H
#ifdef STATE_MACHINE_C
void StateMachine_loop(stateFunction entryPoint) {
  stateNext next = entryPoint(nullUmf);
  while (next.sf) {
    next = (stateNext)(((stateFunction)(next.sf))(next.caller));
  }
  // return nullNext to end
}
#endif // STATE_MACHINE_C
