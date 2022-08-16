"""A single CPU is built based on the given frequency and number of instructions"""

import common

class CPU:
    def __init__(self, frequency, number_of_instructions, pipeline_stages):

        self.frequency = self._is_valid_freq(frequency)
        self.number_of_instructions = self._is_valid_number(number_of_instructions)
        self.pipeline_stages = pipeline_stages
        self.voltage = common.json_data["voltage_range"][str(self.frequency)][0]
        self.current = common.json_data["voltage_range"][str(self.frequency)][1]
        self.cycle_time = (1 / self.frequency) # (us)
        self.power_task = self._power_cpu(self.voltage, self.current) # (uW)
        self.time_total = (self._time_total_cpu(self.pipeline_stages, self.number_of_instructions, self.cycle_time)) # (us)
        self.energy = ((self.power_task * self.time_total) / 1000) # (nJ)

    def _power_cpu(self, v, i):
        return v * i

    def _energy_cpu(self, p, T):
        return p * T

    def _time_total_cpu(self, k, n, t):
        return (k + n + 1) * t

    def _clock_time_cpu(self, f):
        return 1 / f

    def _is_valid_freq(self, f):
        if f < 1:
            pass #raise ValueError("Minimum frequency is 1 MHz")
        return f

    def _is_valid_number(self, n):
        if n < 1:
            raise ValueError("Minimum number of instructions is 1")
        return n