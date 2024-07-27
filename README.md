#  Command line directory integrity control tool FIMO (File Integrity MOnitoring)


## What is FIMO?
FIMO is simple directory integrity control tool written in C for educational purposes. It's compatible only with Linux systems due to usage of platform dependent C header files. Most of it error and info messages are defined in russian language.

FIMO can set contol-file of directory to further integrity check:
```./fimo [-s set] 'path/to/dir' 'path/to/control-file```

By using existing control-file FIMO can check directory integrity:
```./fimo [-c check] 'path/to/dir' 'path/to/control-file```
