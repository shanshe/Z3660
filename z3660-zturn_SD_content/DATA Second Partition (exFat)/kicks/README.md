# KICKSTART FILES
Put in this directory all kickstarts images that you want to use when using CPU emulation. (Not yet for real CPU, work in progress).

Remember their names, you will need to name them exactly in the z3660cfg.txt file.

There is a way of dumping your real kickstart to a file, but it is done manually using some ARM commented out code. Don't know if it is necessary anyway, as everyone has they're own collection of kickstart files for a lot of machines, emulators, etc...

Please, if you want to use 3.1 kickstart, remember that a 060 doesn't boot with original 3.1 kickstart. You need a patched kickstart to disable FPU... You can of course use 3.1.4 or 3.2.x kickstarts files.