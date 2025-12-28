database -open waves -into waves.shm -default
catch {probe -create -shm -all -variables -depth 1} msg

run
exit
