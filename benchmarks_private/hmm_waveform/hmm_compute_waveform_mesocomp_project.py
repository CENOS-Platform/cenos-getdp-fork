# Build lists of downscaled quantities per key
#=============================================
list_time = '0'
list_ax = '0'
list_ay = '0'
list_az = '0'
list_bx = '0'
list_by = '0'
list_bz = '0'
list_dtax = '0'
list_dtay = '0'
list_dtaz = '0'
list_dtbx = '0'
list_dtby = '0'
list_dtbz = '0'
for j in range(0,len_time):
    getdp_list_key = (key[0], key[1], time_table[j+1])
    list_time += " " + str(time_table[j+1])
    list_bx += " " + str(bx_table[getdp_list_key])
    list_by += " " + str(by_table[getdp_list_key])
    list_bz += " " + str(bz_table[getdp_list_key])

print 'Done projecting for mesoscale problems ' + str(key[0])
file_name      = "res_meso/az_pert_Prob_Elenum%g_TW%g_Yose.pos" % (key[0], (which_time_window) ) # used with GmshRead
key_proj       = (key[0], key[1], time_table[which_time_step])
args.extend([file_dir + "getdp.sh", file_dir + "smc_meso_waveform",
             "-bin", "-v", "2", "-v2", "-solve", "a_NR_WR", 
             "-pos", "mean_WR", "map_field_WR",
             "-setnumber", "Flag_Dynamic", str(flag_dynamic),
             "-setnumber", "Freq", str(freq),
             "-setnumber", "t_0", str(t_0),
             "-setnumber", "t_end", str(t_end),
             "-setnumber", "dt_Macro", str(dt_Macro),
             "-setnumber", "NbSteps", str(nb_steps),
             "-setlist", "TIMEVEC "  + str(len_time), str(list_time),
             #"-setlist", "BX " + str(len_time), str(list_bx),
             #"-setlist", "BY " + str(len_time), str(list_by),
             #"-setlist", "BZ " + str(len_time), str(list_bz),
             "-setnumber", "BX ", str(bx_table[key_proj]),                      
             "-setnumber", "BY ", str(by_table[key_proj]),                      
             "-setnumber", "BZ ", str(bz_table[key_proj]),                      
             "-setnumber", "ELENUM", str(key[0]),
             "-setnumber", "QPINDEX", str(key[1]),
             "-gmshread", file_name,
             "-setnumber", "which_time_window", str(which_time_window),
             "-setnumber", "which_wr_iteration", str(which_wr_iteration),
             "-setnumber", "which_time_step", str(which_time_step),
             "-setnumber", "which_time", str(which_time),
             "-setnumber", "len_time", str(len_time),
             "-setnumber", "Flag_WR_meso", str(Flag_WR_meso)])
cpus[i] = subprocess.Popen(args)
