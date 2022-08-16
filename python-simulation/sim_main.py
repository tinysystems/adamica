import common

import charts as chrts
import sstm as sstm

def main():

    common.init()
        
    num_cores = common.json_data["system_config"]["num_cores"]
    num_instr = common.json_data["system_config"]["num_instr"]
    master_frac = common.json_data["system_config"]["master_frac"]
    master_freq = common.json_data["system_config"]["master_freq"]
    slave_freq = common.json_data["system_config"]["slave_freq"]
    pipeline_stages = common.json_data["system_config"]["pipeline_stages"]
    capacitor_size = common.json_data["system_config"]["capacitor_size"]
    
    sys = sstm.Sstm(capacitor_size, num_cores, num_instr, pipeline_stages, master_frac, master_freq, slave_freq) # Deploy a singlecore system
    ch = chrts.Chart(sys) # Add the single-core system as a base system to the chart

    ch.Pin_Ncores_Tsystem_3D_show()

    ch.Pin_Ncores_show()

    ch.Pin_Speedup_show()

    ch.Pin_Enrg_show()

if __name__ == "__main__":
    main()

