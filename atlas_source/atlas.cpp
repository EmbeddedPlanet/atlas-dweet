
#include "mbed.h"
#include "atlas.h"

void init_atlas(void)
{

    if (NRF_UICR->REGOUT0 != UICR_REGOUT0_VOUT_3V3) 
    {
        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}
        NRF_UICR->REGOUT0 = UICR_REGOUT0_VOUT_3V3;
        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}
    }

}