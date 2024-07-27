#  Command line directory integrity monitoring tool FIMO (File Integrity MOnitoring)

## What is FIMO?
FIMO is simple directory integrity monitoring tool written in C for educational purposes. It's compatible only with Linux systems due to usage of platform dependent C header files. Most of it error and info messages are defined in russian language.

FIMO can set contol-file of directory to further integrity check:

```./fimo [-s set] 'path/to/dir' 'path/to/control-file```

By using existing control-file FIMO can check directory integrity:

```./fimo [-c check] 'path/to/dir' 'path/to/control-file```

## How it works?
While filling in directory control-file FIMO recursively goes through all subdirectories and collects all filepaths. Program calculates hash of every file using `openSSL` SHA256 hashing algorithm and writes it into control-file.
Control-files are simple `.txt` files that should not be modified by user directly. FIMO performs control-file validation that can only track format mismatch.

To check directory integrity FIMO creates temporary file with current contents of directory and compares it with filepaths and hashes storeg in passed control-file.

Types of mismatch:
* Errors
  - file was removed from directory
  - file was added to directory
  - file was changed
* Warnings
  - file was renamed or replaced with copy with different name

## Implementation features
- Control-files can not be stored in the directory during `-set` and `-check` commands
- We deal with `n` files in directory and `m` files in control list, so comparison algorithm has complexity of `O(2*n*m + n + m)` and should be reworked

## Installation
1. Instal OpenSSL:
```sudo apt install libssl-dev```
2. Clone repository
3. Build project:
```make```
