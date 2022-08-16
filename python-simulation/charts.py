"""Shows different charts on demand"""

import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import sys

import common
import sstm as sstm


class Chart:
    def __init__(self, base_sys):
        self.base_sys = self._is_valid_base_system(base_sys)
        self.base_core = self.base_sys.cores[0]

    def _is_valid_base_system(self, base_sys):
        if len(base_sys.cores) != 1:
            raise ValueError("The base system has to be always a siglecore system")
        return base_sys

    def _deploy_system(self, capacitor_size, number_of_cores, number_of_instructions, pipeline_stages, m_frac, m_freq, s_freq=None, powin=None):
        return sstm.Sstm(capacitor_size, number_of_cores, number_of_instructions, pipeline_stages, m_frac, m_freq, s_freq, powin)

    """Compares energy and time consumption. Singlecore vs Multicore with different workload. Homogeneous system."""
    def Pin_Ncores_Tsystem_3D(self, n, p, 
                            capacitor_size=None, 
                            master_frac=None, 
                            time=True, benefit=False, energy=False):
        res = []
        pin = []
        ncor = []
        ben = []
        enrg = []
        
        if capacitor_size is None:
            capacitor_size = common.json_data["capacitor_size"]["default"]
        if master_frac is None:
            master_frac = common.json_data["parallel_fraction"]["default"]

        min_n = 1
        singcore_time = 0
        singcore_enrg = 0
        for i in range(len(n)):
            tmp = []
            min_time = sys.maxsize
            min_enrg = sys.maxsize
            for j in range(len(n[0])):
                master_fraction = master_frac
                master_frequency = common.json_data["system_config"]["master_freq"]
                slave_frequency = common.json_data["system_config"]["slave_freq"]
                number_instruction = common.json_data["system_config"]["num_instr"]
                pipeline_stages = common.json_data["system_config"]["pipeline_stages"]
                if n[i][j] == 1: 
                    master_fraction = 100
                target_sys = self._deploy_system(capacitor_size, n[i][j], 
                                                number_instruction, 
                                                pipeline_stages, 
                                                master_fraction, 
                                                master_frequency, 
                                                slave_frequency, p[i][j])
                tmp_time = round(target_sys.get_total_time())
                tmp_enrg = round(target_sys.get_total_energy())
                if n[i][j] == 1:
                    singcore_time = tmp_time 
                    singcore_enrg = tmp_enrg
                if (tmp_time > 0 and tmp_enrg > 0 and min_time > tmp_time): 
                    min_time = tmp_time
                    min_enrg = tmp_enrg
                    min_n = n[i][j]
                #if (n[i][j] > 1 and tmp_enrg > 0 and min_enrg > tmp_enrg): 
                    #min_enrg = tmp_enrg
                tmp.append(tmp_time)
                del target_sys
            pin.append(p[i][j])
            ncor.append(min_n)
            ben.append((1 - min_time/singcore_time) * 100)
            enrg.append((1 - min_enrg/singcore_enrg) * 100)
            res.append(tmp)
        #print(ideal)
        if time:
            return np.array(res)
        elif benefit:
            return pin, ben, enrg
        elif energy:
            return pin, enrg
        else:
            return pin, ncor

    def Pin_Ncores_Tsystem_3D_show(self):
        #===============
        #  The main plot
        #===============
        n = 1 + 8
        low_Pin = 50
        high_Pin = 50000

        fig_main = plt.figure()
        ax_main = fig_main.gca(projection='3d')

        # Make data
        Pin = np.arange(low_Pin, high_Pin, 100)
        N = np.arange(1, n, 1)

        N, Pin = np.meshgrid(N, Pin)
        T = self.Pin_Ncores_Tsystem_3D(N, Pin)
        #T = minmax_scale(T, feature_range=(0,1000000)) # Data normalization

        # Plot the surface
        surf = ax_main.plot_surface(Pin, N, T, color='c', #cmap='Set2',
                            antialiased=False)

        # Customize the z axis
        ax_main.set_zlim(0, 2000000)
        #ax_main.zaxis.set_major_locator(LinearLocator(10))
        #ax_main.zaxis.set_major_formatter(FormatStrFormatter('%.0f'))
        ax_main.set_zlabel("Time (us)", labelpad=10, rotation='vertical')
        ax_main.set_ylabel("# of cores", labelpad=10, rotation='vertical')
        ax_main.set_xlabel("Input Power (uW)", labelpad=10, rotation='vertical')
        M = np.arange(1, n, 1)
        ax_main.set_yticks(np.arange(min(M), max(M)+1, 2.0))

        # Add a color bar which maps values to colors.
        #fig_main.colorbar(surf, shrink=0.5, aspect=5)

        # rotate the axes and update
        ax_main.view_init(30, -20)
        plt.draw()

        #fig_main.savefig("3d_whole_surfc.pdf", bbox_inches='tight')
        #plt.show()

        #===============
        #  First subplot
        #===============

        # set up a figure twice as wide as it is tall
        #fig = plt.figure(figsize=(12,4))
        #fig.subplots_adjust(right=0.95, left=0.0, wspace=0.3, top=0.98, bottom=0.0)
        #ax = fig.add_subplot(1, 3, 1, projection='3d', title='(a)')

        fig = plt.figure()
        ax = fig.gca(projection='3d')

        Pin = np.arange(low_Pin, 5000, 100)
        N = np.arange(1, n, 1)

        N, Pin = np.meshgrid(N, Pin)
        T  = self.Pin_Ncores_Tsystem_3D(N, Pin)

        surf = ax.plot_wireframe(Pin, N, T, color='c',
                            linewidth=1, antialiased=False)

        # Customize the z axis
        ax.set_zlim(0, 2000000)
        #ax.zaxis.set_major_locator(LinearLocator(10))
        #ax.zaxis.set_major_formatter(FormatStrFormatter('%.0f'))
        ax.set_zlabel("Time (us)", labelpad=10) #, fontsize=5)
        ax.set_ylabel("# of cores", labelpad=10) #, fontsize=5)
        ax.set_xlabel("Input Power (uW)", labelpad=10) #, fontsize=5)
        M = np.arange(1, n, 1)
        ax.set_yticks(np.arange(min(M), max(M)+1, 2.0))

        # Add a color bar which maps values to colors.
        #fig.colorbar(surf, shrink=0.5, aspect=5)


        # rotate the axes and update
        ax.view_init(30, -50)
        #ax.tick_params('both', labelsize=5, pad=0)
        plt.draw()
        #fig.savefig("3d_part_1c.pdf", bbox_inches='tight')

        #===============
        # Second subplot
        #===============
        # set up the axes for the second plot
        #ax = fig.add_subplot(1, 3, 2, projection='3d', title='(b)')

        fig = plt.figure()
        ax = fig.gca(projection='3d')

        Pin = np.arange(5000, 35000, 100)
        N = np.arange(1, n, 1)

        N, Pin = np.meshgrid(N, Pin)
        T  = self.Pin_Ncores_Tsystem_3D(N, Pin)

        surf = ax.plot_wireframe(Pin, N, T, color='c',
                            linewidth=1, antialiased=False)

        # Customize the z axis
        ax.set_zlim(0, 25000)
        #ax.zaxis.set_major_locator(LinearLocator(10))
        #ax.zaxis.set_major_formatter(FormatStrFormatter('%.0f'))
        ax.set_zlabel("Time (us)", labelpad=10) #, fontsize=5)
        ax.set_ylabel("# of cores", labelpad=10) #, fontsize=5)
        ax.set_xlabel("Input Power (uW)", labelpad=10) #, fontsize=5)
        M = np.arange(1, n, 1)
        ax.set_yticks(np.arange(min(M), max(M)+1, 2.0))

        # Add a color bar which maps values to colors.
        #fig.colorbar(surf, shrink=0.5, aspect=5)

        # rotate the axes and update
        ax.view_init(30, -50)
        #ax.tick_params('both', labelsize=5, pad=0)
        plt.draw()
        #fig.savefig("3d_part_2c.pdf", bbox_inches='tight')

        #===============
        # Third subplot
        #===============
        # set up the axes for the second plot
        #ax = fig.add_subplot(1, 3, 3, projection='3d', title='(c)')

        fig = plt.figure()
        ax = fig.gca(projection='3d')

        Pin = np.arange(35000, high_Pin, 100)
        N = np.arange(1, n, 1)

        N, Pin = np.meshgrid(N, Pin)
        T = self.Pin_Ncores_Tsystem_3D(N, Pin)

        surf = ax.plot_wireframe(Pin, N, T, color='c',
                            linewidth=1, antialiased=False)

        # Customize the z axis
        ax.set_zlim(0, 9000)
        #ax.zaxis.set_major_locator(LinearLocator(10))
        #ax.zaxis.set_major_formatter(FormatStrFormatter('%.0f'))
        ax.set_zlabel("Time (us)", labelpad=10) #, fontsize=5)
        ax.set_ylabel("# of cores", labelpad=10) #, fontsize=5)
        ax.set_xlabel("Input Power (uW)", labelpad=10) #, fontsize=5)
        M = np.arange(1, n, 1)
        ax.set_yticks(np.arange(min(M), max(M)+1, 2.0))

        # Add a color bar which maps values to colors.
        #fig.colorbar(surf, shrink=0.5, aspect=5)

        # rotate the axes and update
        ax.view_init(30, -20)
        plt.draw()
        #ax.tick_params('both', labelsize=5, pad=0)
        #fig.savefig("3d_part_3c.pdf", bbox_inches='tight')

        plt.show()

    def Pin_Ncores_show(self):

        matplotlib.rcParams.update({'font.size': 14})

        min_cap = common.json_data["capacitor_size"]["min"]
        max_cap = common.json_data["capacitor_size"]["max"]
        step_cap = common.json_data["capacitor_size"]["step"]

        capacitors = np.arange(min_cap, max_cap, step_cap) # The range of capacitances (uF)
        m = 1 + 8 #number of cores
        # Generate data
        legend_array = []
        for i in capacitors:
            legend_array.append(str(i) + "uF")
            
        Pin = np.arange(50, 50000, 100)
        N = np.arange(1, m, 1)
        N, Pin = np.meshgrid(N, Pin)

        colors = ['b', 'g', 'r', 'y', 'm']
        lines = [[3,0], [3,3], [5,5], [3,1,1,1,3], [1,1]]

        for i in range(len(capacitors)):
            p, n = self.Pin_Ncores_Tsystem_3D(N, Pin, capacitor_size=capacitors[i], time=False)
            plt.plot(p, n, dashes=lines[i], color=colors[i])
        #plt.axhline(0, color='k')
        plt.xlabel('Input Power (uW)')
        plt.ylabel('# of cores')
        M = np.arange(1, 7, 1)
        plt.yticks(np.arange(min(M), max(M)+1, 1.0))
        plt.legend(["20uF", "30uF", "40uF", "50uF", "60uF"])
        #plt.savefig("2d_pin_ncor.pdf")
        plt.show()

    def Pin_Speedup_show(self):
        matplotlib.rcParams.update({'font.size': 14})

        min_cap = common.json_data["capacitor_size"]["min"]
        max_cap = common.json_data["capacitor_size"]["max"]
        step_cap = common.json_data["capacitor_size"]["step"]

        capacitors = np.arange(min_cap, max_cap, step_cap) # The range of capacitances (uF)
        
        # Generate data
        legend_array = []
        for i in capacitors:
            legend_array.append(str(i) + "uF")

        Pin = np.arange(50, 50000, 100)
        N = np.arange(1, 9, 1)
        N, Pin = np.meshgrid(N, Pin)

        colors = ['b', 'g', 'r', 'y', 'm']
        lines = [[3,0], [3,3], [5,5], [3,1,1,1,3], [1,1]]

        ax1 = plt.subplot(211)
        ax2 = plt.subplot(212)
        for i in range(len(capacitors)):
            p, b, e = self.Pin_Ncores_Tsystem_3D(N, Pin, capacitors[i], time=False, benefit=True)
            ax1.plot(p, b, dashes=lines[i], color=colors[i])
            ax2.plot(p, e, dashes=lines[i], color=colors[i])

        #plt.axhline(0, color='k')
        ax2.set_xlabel('Input Power (uW)')
        ax1.set_ylabel('Performance (%)')
        ax2.set_ylabel('Enrg eff-cy (%)')
        ax1.legend(["20uF", "30uF", "40uF", "50uF", "60uF"], bbox_to_anchor=(0., 1.02, 1., .102), loc='lower left', ncol=5, mode="expand", borderaxespad=0.)
        #plt.savefig("2d_pin_speedup.pdf")
        #plt.savefig("2d_pin_speedup_energy_cap.pdf")
        plt.show()

    def Pin_Enrg_show(self):

        min_frac = common.json_data["parallel_fraction"]["min"]
        max_frac = common.json_data["parallel_fraction"]["max"]
        step_frac = common.json_data["parallel_fraction"]["step"]

        p_factors = np.arange(min_frac, max_frac, step_frac)
        # Generate data
        legend_array = []
        for i in p_factors:
            legend_array.append(str(i) + "Seq")

        Pin = np.arange(50, 50000, 100)
        N = np.arange(1, 9, 1)
        N, Pin = np.meshgrid(N, Pin)

        colors = ['b', 'g', 'r', 'y', 'm']
        lines = [[3,0], [3,3], [5,5], [3,1,1,1,3], [1,1]]

        ax1 = plt.subplot(211)
        ax2 = plt.subplot(212)
        for i in range(len(p_factors)):
            p, b, e = self.Pin_Ncores_Tsystem_3D(N, Pin, capacitor_size=40, master_frac=p_factors[i], time=False, benefit=True)
            ax1.plot(p, b, dashes=lines[i], color=colors[i])
            ax2.plot(p, e, dashes=lines[i], color=colors[i])

        #plt.axhline(0, color='k')
        ax2.set_xlabel('Input Power (uW)')
        ax1.set_ylabel('Performance (%)')
        ax2.set_ylabel('Enrg eff-cy (%)')
        ax1.legend(legend_array, bbox_to_anchor=(0., 1.02, 1., .102), loc='lower left', ncol=5, mode="expand", borderaxespad=0.)
        #plt.savefig("2d_pin_speedup_energy_seq.pdf")
        plt.show()
