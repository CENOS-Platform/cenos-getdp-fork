print("Python: initializing HMM tables");

currentTime     = input[0]
currentTimeStep = input[1]

# python input (filled during downscaling)
ax_table = {}
ay_table = {}
az_table = {}
bx_table = {}
by_table = {}
bz_table = {}
dtax_table = {}
dtay_table = {}
dtaz_table = {}
dtbx_table = {}
dtby_table = {}
dtbz_table = {}

# python output (filling after meso computations and used for upscaling)
hx_table = {}
hy_table = {}
hz_table = {}
dhdbxx_table = {}
dhdbxy_table = {}
dhdbxz_table = {}
dhdbyx_table = {}
dhdbyy_table = {}
dhdbyz_table = {}
dhdbzx_table = {}
dhdbzy_table = {}
dhdbzz_table = {}
JouleLosses_table = {}
MagneticPower_table = {}
MagneticEnergy_table = {}
