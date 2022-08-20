/*
 *
 * Authors 2021 Khakim Akhunov and Kasim Sinan Yildirim
 *
 *
 *
 */

#ifndef FRAM_H_
#define FRAM_H_



/* defines non-volatile variable */
#ifdef __GNUC__
    #define _intfram    __attribute__((section(".nv_vars")))
#else
    #define _intfram __attribute__((section(".TI.persistent")))
#endif

#endif /* FRAM_H_ */
