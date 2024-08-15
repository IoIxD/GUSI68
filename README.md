# GUSI68

GUSI (Grand Unified Socket Interface) is a POSIX/Pthreads/Sockets library bringing some of the comforts of UNIX 98 to classic MacOS (7.0+). GUSI68 is a fork that lets it be used with Retro68.

**Notes:**

- Using this as is requires commenting out several things in your Retro68 installation.
- Networking does not work, because
  - The person who wrote it used several horrifically cursed C++ practices that were accepted by MetroWerks but not by gcc.
  - It relies on an "dnr.c" file from Apple for the "OpenResolver" function, which throws a compiler error that I can't seem to solve. Maybe [the person I got it from](https://github.com/antscode/MacTCPHelper/) was using the reverse engineered headers and not the Universal headers like I am? Don't know.

Currently does not build, but the CodeWarrior files (which are currently kept in original_nw) are fully converted to sane .cpp/.h files.
