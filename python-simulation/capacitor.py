
class Capacitor:
    def __init__(self, size):
        self.energy_stored = ((3.6 * 3.6 * size) / 2) * 1000 # (nJ) Ecap = (V^2*C)/2
    
    def get_recharge_time(self, power_input):
        if power_input <= 0:
            raise ValueError("The capacitor is not powered.")

        self.power_input = power_input
        return (self.energy_stored / self.power_input) * 1000 # (us)

    def get_discharge_time(self, system_energy):
        return self.energy_stored / system_energy