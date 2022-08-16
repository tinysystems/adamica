import numpy as np


"""A single/multicore system is built based on the given number of cores, workload distribution, and type"""
"""fraction - %, frequency - MHz"""
import common
import cpu as cpu
import capacitor as cap

class Sstm:

    def __init__(self, capacitor_size, number_of_cores, number_of_instructions, pipeline_stages, master_fraction, master_frequency, slave_frequency=None, power_input=None):
        self.number_of_cores = self._is_valid_number(number_of_cores)
        self.number_of_instructions = number_of_instructions
        self.pipeline_stages = pipeline_stages
        
        self._calculate_fractions(master_fraction)
        
        self.master_frequency = master_frequency
        if slave_frequency is None:
            slave_frequency = master_frequency
        self.slave_frequency = slave_frequency
        
        self.cores = []
        self._assign_cores()

        self.capacitor_size = capacitor_size # (uF) self.get_total_energy()
        if power_input is None: 
            self.power_input = 5000 # (uW) from an energy harvester
        else:
            self.power_input = power_input
        self._specify_capacitor()

        self.checkpoint_time = common.json_data["overhead_config"]["checkpoint_time"] # (us)
        self.boot_time = common.json_data["overhead_config"]["boot_time"] # (us)
        self.com_time = common.json_data["overhead_config"]["com_time"] # (us)
        self.checkpoint_energy = common.json_data["overhead_config"]["checkpoint_energy"] # (nJ)
        self.boot_energy = common.json_data["overhead_config"]["boot_energy"] # (nJ)
        self.com_energy = common.json_data["overhead_config"]["com_energy"] # (nJ)
        self.leakage_power = common.json_data["overhead_config"]["leakage_power"] # (uW) Data from MSP430 datasheet. V and I at LPM4 
        

    def _is_valid_number(self, n):
        if n < 1:
            raise ValueError("Minimum number of cores is 1")
        return n

    def _is_valid_fraction(self, frac):
        if frac < 0 or frac > 100:
            raise ValueError("The range of the fraction is 0-100%")
        if self.number_of_cores == 1 and frac < 100:
            raise ValueError("Total workload is less then 100%. Check number of cores and master fraction")
        return frac

    def _calculate_fractions(self, master_fraction):

        self.master_fraction = self.number_of_instructions * self._is_valid_fraction(master_fraction) / 100 # Convert the fraction into number of instructions
        if master_fraction < 100:
            self.slave_fraction = self.number_of_instructions * ((100 - master_fraction) / (self.number_of_cores)) / 100 # Convert the fractions into number of instructions
        else:
            self.slave_fraction = 0
    
    def _assign_cores(self):
        for i in range(self.number_of_cores):
            if i == 0:
                self.cores.append(cpu.CPU(self.master_frequency, self.master_fraction, self.pipeline_stages))
            else:
                self.cores.append(cpu.CPU(self.slave_frequency, self.slave_fraction, self.pipeline_stages))
    
    def _specify_capacitor(self):
        self.capacitor = cap.Capacitor(self.capacitor_size)
    
    def get_total_time(self):
        recharge_time = self.capacitor.get_recharge_time(self.power_input)
        energy_part = self._get_energy_part() 
        
        total_time = self.get_task_time() + (recharge_time + self.checkpoint_time + self.boot_time) * energy_part
        if self.number_of_cores > 1:
            total_time += self.com_time
        
        return total_time

    def get_task_time(self):
        task_time = self.cores[0].time_total # Master core time. Master always the first in the list
        if self.number_of_cores > 1: 
            task_time += self.cores[1].time_total # Slave core time
        return task_time

    def _get_energy_part(self):
        energy = 0
        if self.number_of_cores > 1:
            energy = (self.get_task_energy() + self.com_energy * (self.number_of_cores - 1)) / (self.capacitor.energy_stored - (self.checkpoint_energy + self.boot_energy) * self.number_of_cores)
        else:
            energy = self.get_task_energy() / (self.capacitor.energy_stored - self.checkpoint_energy - self.boot_energy)
        
        return energy
    def get_total_energy(self):
        energy = 0
        if self.number_of_cores > 1:
            energy = (self.get_task_energy() + self.com_energy * (self.number_of_cores - 1)) / (1 - ((self.checkpoint_energy + self.boot_energy) * self.number_of_cores) / self.capacitor.energy_stored)
        else:
            energy = self.get_task_energy() / (1 - (self.checkpoint_energy + self.boot_energy) / self.capacitor.energy_stored)
        return energy

    def get_task_energy(self):
        energy = self.cores[0].power_task * self.get_task_time()
        if self.number_of_cores > 1:
            energy += (self.cores[1].power_task * self.cores[1].time_total + self.leakage_power * self.cores[0].time_total) * (self.number_of_cores - 1)
        return energy / 1000

    def get_total_power(self):
        power = 0
        for core in self.cores:
            power += core.power_task
        return power
