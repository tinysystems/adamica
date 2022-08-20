#ifndef ADAMICA_H_
#define ADAMICA_H_

#include <msp430.h>
#include <stdarg.h>
#include <stdint.h>
#include <extfram.h>

#define MAINCORE

#ifdef MAINCORE
#define mainCore 1
#define coreid 0
#else
#define mainCore 0
#define coreid 1
#endif

#define is_valid(x) (x == 0 ? 1 : 0)
#define EXEC_CMD 0xE0
#define FIN_CMD 0xF0
#define _begin_parallel(func) if(mainCore) adamica(func);
#define _end_parallel if(mainCore) waitCores(); else { signalMainCore(); deepSleep(); }

typedef void (*fn_ptr)();

extern uint8_t numCores;
extern uint8_t dmt[];
extern EXTFRAM_ADDR baseAddress;
extern EXTFRAM_ADDR finAddress;

/* Function: adamica
 * -----------------
 * samples input power, makes a decision on the number of cores to activate,
 * and, if necessary, activates secondary core(s) to run a specified function
 * on specified data
 *
 * fn_ptr: pointer to the function to be executed on secondary core(s)
 *
 * returns: void
 */
void adamica(fn_ptr);

/* Function: waitCores
 * -----------------
 * enables an interrupt trigger and goes to a low-power mode
 *
 * returns: void
 */
void waitCores();

/* Function: signalMainCore
 * -----------------
 * triggers a pin to interrupt the main core to announce
 * the completion of the assigned task.
 * Note: the function is executed by a secondary core(s)
 *
 * returns: void
 */
void signalMainCore();

/* Function: deepSleep
 * -----------------
 * enables an interrupt transitions a core to the lowest low-power mode
 *
 * returns: void
 */
void deepSleep();

/* Function: sampleADC
 * -----------------
 * samples ADC that is connected to an energy harvester.
 * The samples values are actual input power levels
 *
 * returns: the absolute input power value in uW
 */
uint16_t sampleADC();

/* Function: DMT
 * -----------------
 * makes a decision with respect to the input power level.
 * The decisions are made based on the Decision Making Table provided in advance
 * or computed at runtime.
 *
 * fn_ptr: pointer to the function to be executed on secondary core(s)
 * uint16_t: input power level in uW
 *
 * returns: the number of cores to activate
 */
uint8_t DMT(uint16_t);

/* Function: activate
 * -----------------
 * triggers a pin(s) of the secondary core(s) to interrupt
 * and switch them from sleep to active mode.
 *
 * uint8_t: the number of cores to activate
 *
 * returns: void
 */
void activate(uint8_t, fn_ptr);




#endif /* ADAMICA_H_ */
