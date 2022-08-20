/*
 *
 * Authors 2021 Khakim Akhunov and Kasim Sinan Yildirim
 *
 *
 *
 */

#ifndef EXTFRAM_H_
#define EXTFRAM_H_

#include <stdint.h>

typedef uint8_t EXTFRAM_ADDR[3];

int generate_address(EXTFRAM_ADDR base_address, int32_t offset, EXTFRAM_ADDR new_address);

void readEXTFRAM(EXTFRAM_ADDR address, uint8_t dst[], uint32_t len);
void writeEXTFRAM(EXTFRAM_ADDR address, uint8_t src[], uint32_t len);
void eraseEXTFRAM(EXTFRAM_ADDR address, uint32_t len);

#endif /* EXTFRAM_H_ */
