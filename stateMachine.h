#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H
// basic state machine

typedef void *(*statefn)();
static statefn statemachine_iterate(statefn initial)
{
  return (statefn)initial;
}
static void statemachine_loop(statefn initial)
{
  statefn state = initial;
  while (state)
  {
    state = (statefn)state();
  }
}

#endif // STATE_MACHINE_H
