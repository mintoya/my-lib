#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H
#include "stringList.h"
#include "um_fp.h"
#include <stdarg.h>
typedef um_fp (*statefunction)(void);
typedef struct {
  um_fp stateName;
  statefunction stateFunction;
} stateMachine_state;
typedef struct {
  // offset by 1, 0 is stop state
  unsigned int currentState;
  stringList *states;
  List *stateFunctions;
} stateMachine;

static _Thread_local List *stateMachine_return_helper = NULL;
static void __attribute__((destructor)) stateMachine_return_helper_cleanup() {
  if (stateMachine_return_helper)
    List_free(stateMachine_return_helper);
}

// not required, but easiest way to own memory
#define SM_RETURN(um)                                                          \
  if (!stateMachine_return_helper)                                             \
    stateMachine_return_helper = mList(uint8_t);                               \
  stateMachine_return_helper->length = 0;                                      \
  List_appendFromArr(stateMachine_return_helper, um.ptr, um.width);            \
  return ((um_fp){.ptr = stateMachine_return_helper->head,                     \
                  .width = List_headArea(stateMachine_return_helper)});

#define nullState                                                              \
  ((stateMachine_state){.stateName = nullUmf, .stateFunction = NULL})
#define SMState(name, function)                                                \
  ((stateMachine_state){.stateName = um_from(name), .stateFunction = function})
#define stateMachine_new(...)                                                  \
  stateMachine_new_f(__VA_ARGS__ __VA_OPT__(, ) nullState)
// null-terminated
stateMachine *stateMachine_new_f(stateMachine_state initialState, ...);
static inline void stateMachine_free(stateMachine *sm) {
  List_free(sm->stateFunctions);
  stringList_free(sm->states);
  free(sm);
}
static inline um_fp stateMachine_getState(stateMachine *sm) {
  if (!sm->currentState)
    return nullUmf;
  else
    return stringList_get(sm->states, sm->currentState - 1);
}
unsigned int stateMachine_iterate(stateMachine *sm);
void stateMachine_loop(stateMachine *sm);
#endif //  STATE_MACHINE_H
#ifdef STATE_MACHINE_C
stateMachine *stateMachine_new_f(stateMachine_state initialState, ...) {
  if (!(initialState.stateFunction && initialState.stateName.width))
    return NULL;
  stateMachine *res = (stateMachine *)malloc(sizeof(stateMachine));
  *res = (stateMachine){
      .currentState = 1,
      .states = stringList_new(),
      .stateFunctions = mList(statefunction),
  };
  va_list args;
  va_start(args, initialState);
  stateMachine_state thisstate = initialState;
  while (thisstate.stateFunction && thisstate.stateName.ptr &&
         thisstate.stateName.width) {
    mList_add(res->stateFunctions, statefunction, thisstate.stateFunction);
    stringList_append(res->states, thisstate.stateName);
    thisstate = va_arg(args, stateMachine_state);
  }
  va_end(args);
  return res;
}
unsigned int stateMachine_iterate(stateMachine *sm) {
  statefunction thisstate =
      mList_get(sm->stateFunctions, statefunction, sm->currentState - 1);
  um_fp nextState = thisstate();
  if (nextState.width) {
    // strinlist returns its length if the item isnt found
    unsigned int next = stringList_search(sm->states, nextState);
    if (next >= stringList_length(sm->states))
      sm->currentState = 0;
    else
      sm->currentState = next + 1;
  } else
    sm->currentState = 0;
  return sm->currentState;
}
void stateMachine_loop(stateMachine *sm) {
  while (stateMachine_iterate(sm))
    ;
}
#endif // STATE_MACHINE_C
