import subprocess
import sys
import os
import time
import socket
import math

#==============================================================
#dt_Macro = 2.0e-7
#==============================================================

nbr_subproblems = input[0]
flag_dynamic = input[1]
freq = input[2]
nb_steps = input[3]
postpro_cuts = input[4]
dt_Macro = input[5]
Flag_WR_meso = input[6]

time_table_keys = time_table.keys()
keys = bx_table.keys()
keys.sort()
#===============================================================
len_time = len(time_table)
len_tables = len(bx_table)
num_mesoproblems = len_tables/len_time
keys_new = []
for i in range(0, num_mesoproblems):
    keys_new.append(((keys[(i)*len_time])[0],(keys[(i)*len_time])[1]))
keys_new.sort()
#===============================================================

#nkeys = len(keys)
nkeys = len(keys_new)
file_dir = os.path.abspath(os.path.dirname(__file__)) + "/"

if os.path.isfile(file_dir + "nodes_pbs.txt"):
    filename = file_dir + "nodes_pbs"
    ssh = "pbsdsh"
elif os.path.isfile(file_dir + "nodes_slurm.txt"):
    filename = file_dir + "nodes_slurm"
    ssh = "srun"
elif os.path.isfile(file_dir + "nodes_ssh.txt"):
    filename = file_dir + "nodes_ssh"
    ssh = "ssh"
else:
    filename = None
    ssh = None

if filename:
    f = open(filename + ".txt")
    nodes = f.readlines()
    ncpus = len(nodes)
    f.close()
else:
    ncpus = 1
    nodes = ["localhost" for x in range(ncpus)]
    f = open(file_dir + "getdp.sh", "w")
    f.write("#!/bin/sh\n{0} $*\n".format(sys.argv[0])) # same getdp as for macro computation
    f.close()
    os.chmod(file_dir + "getdp.sh", 0755)

print("Python: running {0} meso calculations on {1} CPUs".format(nkeys, ncpus))
queue = set(keys_new)
#queue = set(keys)
cpus = ['' for cpu in range(ncpus)]
while len(queue):
    for i, cpu in enumerate(cpus):
        # check if cpu is busy
        if cpu:
            cpus[i].poll()
        # if not busy, launch new calculation
        if not cpu or cpu.returncode != None:
            key = queue.pop()
            args = [];
            if ssh:
                node = nodes[i].strip()
                args.extend([ssh])
                if "pbsdsh" in ssh:
                    args.extend(["-n", str(i), "--"])
                elif "srun" in ssh:
                    args.extend(["-N", "1", "-n", "1", "-w", node])
                else:
                    args.extend([node])
                    
#=========================================================================================                    
            if (Flag_WR_meso != 0):
                #=============================================
                # Build lists of downscaled quantities per key
                #=============================================
                list_time = ''
                list_ax = ''
                list_ay = ''
                list_az = ''
                list_bx = ''
                list_by = ''
                list_bz = ''
                list_dtax = ''
                list_dtay = ''
                list_dtaz = ''
                list_dtbx = ''
                list_dtby = ''
                list_dtbz = ''
                for j in range(0,len_time):
                    getdp_list_key = (key[0], key[1], time_table[j+1])
                    list_time += " " + str(time_table[j+1])
                    list_ax += " " + str(ax_table[getdp_list_key])
                    list_ay += " " + str(ay_table[getdp_list_key])
                    list_az += " " + str(az_table[getdp_list_key])
                    list_bx += " " + str(bx_table[getdp_list_key])
                    list_by += " " + str(by_table[getdp_list_key])
                    list_bz += " " + str(bz_table[getdp_list_key])
                    list_dtax += " " + str(dtax_table[getdp_list_key])
                    list_dtay += " " + str(dtay_table[getdp_list_key])
                    list_dtaz += " " + str(dtaz_table[getdp_list_key])
                    list_dtbx += " " + str(dtbx_table[getdp_list_key])
                    list_dtby += " " + str(dtby_table[getdp_list_key])
                    list_dtbz += " " + str(dtbz_table[getdp_list_key])
                #==================================================================                    
                args.extend([file_dir + "getdp.sh", file_dir + "smc_meso_waveform", 
                             "-bin", "-v", "4", "-v2", "-solve", "a_NR", 
                             "-pos", "mean_1", "mean_2", "mean_3", 
                             "mean_4" if nbr_subproblems == 4 else "",
                             "map_field_1",
                             #"map_field_1" if postpro_cuts else "",
                             #"map_field_1", "map_field_2", "map_field_3",
                             "-setnumber", "Flag_Dynamic", str(flag_dynamic),
                             "-setnumber", "Freq", str(freq),
                             "-setnumber", "dt_Macro", str(dt_Macro),
                             "-setnumber", "NbSteps", str(nb_steps),
                             "-setlist", "TIMEVEC "  + str(len_time), str(list_time),
                             "-setlist", "AX " + str(len_time), str(list_ax),
                             "-setlist", "AY " + str(len_time), str(list_ay),
                             "-setlist", "AZ " + str(len_time), str(list_az),
                             "-setlist", "BX " + str(len_time), str(list_bx),
                             "-setlist", "BY " + str(len_time), str(list_by),
                             "-setlist", "BZ " + str(len_time), str(list_bz),
                             "-setlist", "DTAX " + str(len_time), str(list_dtax),
                             "-setlist", "DTAY " + str(len_time), str(list_dtay),
                             "-setlist", "DTAZ " + str(len_time), str(list_dtaz),
                             "-setlist", "DTBX " + str(len_time), str(list_dtbx),
                             "-setlist", "DTBY " + str(len_time), str(list_dtby),
                             "-setlist", "DTBZ " + str(len_time), str(list_dtbz),                         
                             "-setnumber", "ELENUM", str(key[0]),
                             "-setnumber", "QPINDEX", str(key[1]),
                             "-setnumber", "Flag_WR_meso", str(Flag_WR_meso)])
                cpus[i] = subprocess.Popen(args)
            elif(Flag_WR_meso == 0):
                #=============================================
                # Build lists of downscaled quantities per key
                #=============================================
                list_time = ''
                list_ax = ''
                list_ay = ''
                list_az = ''
                list_bx = ''
                list_by = ''
                list_bz = ''
                list_dtax = ''
                list_dtay = ''
                list_dtaz = ''
                list_dtbx = ''
                list_dtby = ''
                list_dtbz = ''
                for j in range(0,len_time):
                    getdp_list_key = (key[0], key[1], time_table[j+1])
                    list_time += " " + str(time_table[j+1])
                    list_ax += " " + str(ax_table[getdp_list_key])
                    list_ay += " " + str(ay_table[getdp_list_key])
                    list_az += " " + str(az_table[getdp_list_key])
                    list_bx += " " + str(bx_table[getdp_list_key])
                    list_by += " " + str(by_table[getdp_list_key])
                    list_bz += " " + str(bz_table[getdp_list_key])
                    list_dtax += " " + str(dtax_table[getdp_list_key])
                    list_dtay += " " + str(dtay_table[getdp_list_key])
                    list_dtaz += " " + str(dtaz_table[getdp_list_key])
                    list_dtbx += " " + str(dtbx_table[getdp_list_key])
                    list_dtby += " " + str(dtby_table[getdp_list_key])
                    list_dtbz += " " + str(dtbz_table[getdp_list_key])
                #==================================================================
                file_name = "res_meso/a_pert_Prob1_Elenum%g.pos" % (key[0])
                args.extend([file_dir + "getdp.sh", file_dir + "smc_meso_waveform_1",
                             "-bin", "-v", "4", "-v2", "-solve", "a_NR_WR", 
                             "-pos", "mean_WR", "map_field_WR",
                             "-setnumber", "Flag_Dynamic", str(flag_dynamic),
                             "-setnumber", "Freq", str(freq),
                             "-setnumber", "dt_Macro", str(dt_Macro),
                             "-setnumber", "NbSteps", str(nb_steps),
                             "-setlist", "TIMEVEC "  + str(len_time), str(list_time),
                             "-setlist", "AX " + str(len_time), str(list_ax),
                             "-setlist", "AY " + str(len_time), str(list_ay),
                             "-setlist", "AZ " + str(len_time), str(list_az),
                             "-setlist", "BX " + str(len_time), str(list_bx),
                             "-setlist", "BY " + str(len_time), str(list_by),
                             "-setlist", "BZ " + str(len_time), str(list_bz),
                             "-setlist", "DTAX " + str(len_time), str(list_dtax),
                             "-setlist", "DTAY " + str(len_time), str(list_dtay),
                             "-setlist", "DTAZ " + str(len_time), str(list_dtaz),
                             "-setlist", "DTBX " + str(len_time), str(list_dtbx),
                             "-setlist", "DTBY " + str(len_time), str(list_dtby),
                             "-setlist", "DTBZ " + str(len_time), str(list_dtbz),                         
                             "-setnumber", "ELENUM", str(key[0]),
                             "-setnumber", "QPINDEX", str(key[1]),
                             "-gmshread", file_name,
                             "-setnumber", "Flag_WR_meso", str(Flag_WR_meso)])
                cpus[i] = subprocess.Popen(args)
                
#=========================================================================================
        if len(queue) == 0:
            break
    # little sleep so we do not use 100% CPU just for polling
    time.sleep(0.001) 

# wait for last jobs to finish (simply poll() and don't use wait() synchronously
# to avoid having zombie processes triggering the clusters' warning systems)
done = False
while not done:
    done = True
    for i, cpu in enumerate(cpus):
        if cpu and cpu.returncode == None:
            done = False
            cpus[i].poll()
    # little sleep so we do not use 100% CPU just for polling
    time.sleep(0.001)

Dir_Meso = file_dir + "res_meso/" 
#Dir_Meso = file_dir + "../../../../project/results/res_meso/"


# 1. Material law maps per element and GP
#========================================
h_1      = {}
h_2      = {}
h_3      = {}
h_4      = {}
b_1      = {}
b_2      = {}
b_3      = {}
b_4      = {}
jl       = {}
me       = {}

for key in keys_new: # keys_new is the number of problems
    for i in range(nbr_subproblems):
        # Magnetic field map
        #===================
        f = open(Dir_Meso + "h" + str(i + 1) + "_" + str(key[0]) + ".txt", "r")
        g = f.readlines()
        l_1 = 1
        l_2 = 1
        l_3 = 1
        l_4 = 1
        for j in g:
            line = j.split()
            aaa = (float(line[0])/dt_Macro)
            if (i == 0):
                if ( ( ( math.ceil(aaa) - aaa) < 1e-15) or ( ( aaa - math.floor(aaa) ) < 1e-15) ):
                    new_key = (key[0], key[1], time_table[l_1])
                    h_1[new_key] = line[1 : len(line)]
                    l_1 += 1
            elif(i == 1):
                if ( ( ( math.ceil(aaa) - aaa) < 1e-15) or ( ( aaa - math.floor(aaa) ) < 1e-15) ):
                    new_key = (key[0], key[1], time_table[l_2])
                    h_2[new_key] = line[1 : len(line)]
                    l_2 += 1
            elif(i == 2):
                if ( ( ( math.ceil(aaa) - aaa) < 1e-15) or ( ( aaa - math.floor(aaa) ) < 1e-15) ):
                    new_key = (key[0], key[1], time_table[l_3])
                    h_3[new_key] = line[1 : len(line)]
                    l_3 += 1
            elif(i == 3):
                if ( ( ( math.ceil(aaa) - aaa) < 1e-15) or ( ( aaa - math.floor(aaa) ) < 1e-15) ):
                    new_key = (key[0], key[1], time_table[k+1])
                    h_4[new_key] = line[1 : len(line)]
                    l_4 += 1
            else:
                print "Reading the mesoproblem denoted " + str(i) + " Abort..."
                sys.exit(0)
        f.close()
            
        # magnetic induction map
        #=======================
        f = open(Dir_Meso + "b" + str(i + 1) + "_" + str(key[0]) + ".txt", "r")
        g = f.readlines()
        l_1 = 1
        l_2 = 1
        l_3 = 1
        l_4 = 1
        for j in g:
            line = j.split()
            aaa = (float(line[0])/dt_Macro)
            if (i == 0):
                if ( ( ( math.ceil(aaa) - aaa) < 1e-15) or ( ( aaa - math.floor(aaa) ) < 1e-15) ):
                    new_key = (key[0], key[1], time_table[l_1])
                    b_1[new_key] = line[1 : len(line)]
                    l_1 += 1
            elif(i == 1):
                if ( ( ( math.ceil(aaa) - aaa) < 1e-15) or ( ( aaa - math.floor(aaa) ) < 1e-15) ):
                    new_key = (key[0], key[1], time_table[l_2])
                    b_2[new_key] = line[1 : len(line)]
                    l_2 += 1
            elif(i == 2):
                if ( ( ( math.ceil(aaa) - aaa) < 1e-15) or ( ( aaa - math.floor(aaa) ) < 1e-15) ):
                    new_key = (key[0], key[1], time_table[l_3])
                    b_3[new_key] = line[1 : len(line)]
                    l_3 += 1
            elif(i == 3):
                if ( ( ( math.ceil(aaa) - aaa) < 1e-15) or ( ( aaa - math.floor(aaa) ) < 1e-15) ):
                    new_key = (key[0], key[1], time_table[l_4])
                    b_4[new_key] = line[1 : len(line)]
                    l_4 += 1
            else:
                print "Reading the mesoproblem denoted " + str(i) + " Abort..."
                sys.exit(0)
                
        f.close()
                
        # Joule losses map
        #=================
        f = open(Dir_Meso + "JouleLosses_" + str(key[0]) + ".txt", "r")
        g = f.readlines()
        l_1 = 1
        for j in g:
            line = j.split()
            aaa = (float(line[0])/dt_Macro)
            if ( ( ( math.ceil(aaa) - aaa) < 1e-15) or ( ( aaa - math.floor(aaa) ) < 1e-15) ):
                new_key = (key[0], key[1], time_table[l_1])
                jl[new_key] = line[1 : len(line)]
                l_1 += 1
        f.close()
    
        # Magnetic energy map
        #====================
        f = open(Dir_Meso + "MagneticEnergy_" + str(key[0]) + ".txt", "r")
        g = f.readlines()
        l_1 = 1
        for j in g:
            line = j.split()
            aaa = (float(line[0])/dt_Macro)
            if ( ( ( math.ceil(aaa) - aaa) < 1e-15) or ( ( aaa - math.floor(aaa) ) < 1e-15) ):
                new_key = (key[0], key[1], time_table[l_1])
                me[new_key] = line[1 : len(line)]
                l_1 += 1
        f.close()

        
# 2. Writing the material law per element/GP/Time
#================================================

for key in keys:
    hx_table[key] = float(h_1[key][0])
    hy_table[key] = float(h_1[key][1])
    hz_table[key] = float(h_1[key][2])
    dhdbxx_table[key] = (float(h_2[key][0]) - float(h_1[key][0])) / (float(b_2[key][0]) - float(b_1[key][0]))
    dhdbxy_table[key] = (float(h_2[key][1]) - float(h_1[key][1])) / (float(b_2[key][0]) - float(b_1[key][0]))
    dhdbyx_table[key] = (float(h_3[key][0]) - float(h_1[key][0])) / (float(b_3[key][1]) - float(b_1[key][1]))
    dhdbyy_table[key] = (float(h_3[key][1]) - float(h_1[key][1])) / (float(b_3[key][1]) - float(b_1[key][1]))
    
    if nbr_subproblems == 4: # perturbation along z in 3D case
        dhdbxz_table[key] = (float(h_2[key][2]) - float(h_1[key][2])) / (float(b_2[key][0]) - float(b_1[key][0]))
        dhdbyz_table[key] = (float(h_3[key][2]) - float(h_1[key][2])) / (float(b_3[key][1]) - float(b_1[key][1]))
        dhdbzx_table[key] = (float(h_4[key][0]) - float(h_1[key][0])) / (float(b_4[key][2]) - float(b_1[key][2]))
        dhdbzy_table[key] = (float(h_4[key][1]) - float(h_1[key][1])) / (float(b_4[key][2]) - float(b_1[key][2]))
        dhdbzz_table[key] = (float(h_4[key][2]) - float(h_1[key][2])) / (float(b_4[key][2]) - float(b_1[key][2]))
    else:
        dhdbxz_table[key] = 0.0
        dhdbyz_table[key] = 0.0
        dhdbzx_table[key] = 0.0
        dhdbzy_table[key] = 0.0
        dhdbzz_table[key] = 0.0

    JouleLosses_table[key] = float(jl[key][0])
    MagneticEnergy_table[key] = float(me[key][0])
    
# for i in range(nbr_subproblems):
#     os.remove(Dir_Meso + "h" + str(i + 1) + "_" + str(key[0]) + ".txt")
#     os.remove(Dir_Meso + "b" + str(i + 1) + "_" + str(key[0]) + ".txt")
# os.remove(Dir_Meso + "JouleLosses_" + str(key[0]) + ".txt")
# os.remove(Dir_Meso + "MagneticEnergy_" + str(key[0]) + ".txt")

print 'material law h1'
print h_1
print 'material law h2'
print h_2
print 'material law h3'
print h_3
print 'material law b1'
print b_1
print 'material law b2'
print b_2
print 'material law b3'
print b_3
print 'material law dhdbxx_table'
print dhdbxx_table
print 'material law dhdbxy_table'
print dhdbxy_table
print 'material law dhdbyx_table'
print dhdbyx_table
print 'material law dhdbyy_table'
print dhdbyy_table
print 'material law'





print 'materlai law bx_table diff'
for keyyy in keys:
    print 'printing bx_diff Pilotetetetetetetetetetetetetetete'
    print keyyy, (bx_table[keyyy] - bx_old_table[keyyy])
    print 'printing by_diff Pilotetetetetetetetetetetetetetete'
    print keyyy, (by_table[keyyy] - by_old_table[keyyy])
