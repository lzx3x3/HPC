#PBS -N jupyter
#PBS -q coc-ice
#PBS -j oe

module use ${CSE6230_DIR}/modulefiles
module load cse6230

JNPORT=`hexdump -e '"%u"' -n 2 /dev/urandom | awk '{printf ($0 % 2000) + 8000}'`
logfile=${PBS_O_WORKDIR}/${PBS_JOBNAME}.${PBS_JOBID%.ice-sched.pace.gatech.edu}.log
host=${HOSTNAME%.pace.gatech.edu}
printf "To connect to your notebook, run the following command on your laptop/workstation:\\n\\n" > ${logfile}
printf "ssh -t -L ${JNPORT}:localhost:${JNPORT} ${USER}@coc-ice.pace.gatech.edu ssh -L ${JNPORT}:localhost:${JNPORT} ${host}\\n\\n" >> ${logfile}
printf "Do not close the terminal where that is running, and open the address below in your browser:\\n\\n" >> ${logfile}
jupyter notebook --no-browser --port=${JNPORT} 2>> ${logfile}
