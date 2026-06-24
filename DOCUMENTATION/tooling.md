# Tig tooling design spec

## Tigins
Tig installer

The idea is to have a ~/tig/ or C:\tig\ folder where all tig related tools and files are stored.
Tigins will be the tool that manages this folder and downloads the latest version of tigc.

You download Tigins from the github releases page. 
you then run `tigins init`
When ran it first checkcs if C:\tig\ or ~/tig/ exists, if not it creates it and sets it to path forever. 
After that it creates a env.txt file in C:\tig\ or ~/tig/ with this data.
```
TIG_HOME=
TIGC_VERSION=
ARCH=
OS=
```

after that it downloads the latest version of tigc for the OS and arch and stores it in C:\tig\ or ~/tig/
after that it downloads the latest version of tpm for the OS and arch and stores it in C:\tig\ or ~/tig/

### Commands of tigins
```bash
tigins init
# initializes the tig environment
tigins update
# updates tigc and tpm
tigins uninstall
# uninstalls tig completely

```

## tpm (Tig Package Manager)
tpm is a package manager for tig. It allows you to install, update, and remove packages.
The core idea for the UX is this: Install once, use everywhere.
The idea is to have a folder in C:\tig\packages\ where every library 
The easy way would be to just have C:\tig\packages and every lib is just a single file, this way we dont have to deal with a hughe directory tree. So we make tpm package a multi file library into a single .tc file

```bash
tpm package <package_name> <files> 
# example
tpm package advmath math.tc vector.tc matrix.tc /utils/utils.tc /helpers/helpers.tc 
# or
tpm package advmath src/* # all tc files in the subfolder and their subfolders 
# linker error if duplicated symbols
# output
cat advmath.tc
# creates C:\path\to\project\advmath.tc
tpm publish advmath 
$> Username: alonsovm
$> Password: ***************
$> Password: ***************
# Push successful!
```
Someone else
```bash
# tpm install <user>/<package>
tpm install alonsovm/advmath
# this fetches advmath and puts it in C:\tig\libs\advmath.tc 
# no dependency tree needed since everything is bundled in one file
# it can now be used by any project
```
Usage in a tig project
```tig
use "stdlib/io.tc"
@use "packages/advmath.tc"

...
```

## Removing
```bash
# tpm remove <package>
tpm remove advmath
# this removes advmath from C:\tig\packages\advmath.tc 
```

### Commands
```bash
# PACKAGE CREATION
tpm package <name> <file1> <file2> ...   # Bundle files into one .tc
tpm publish <name>                        # Upload to registry, ask for auth

# PACKAGE MANAGEMENT
tpm install <user>/<package>              # Install from registry
tpm remove <package>                      # Remove from system
tpm list                                  # List installed packages
tpm update <package>                      # Update to latest version
tpm search <term>                         # Search registry

# PROJECT MANAGEMENT
tpm init                                  # Create new tpm project
tpm add <user>/<package>
```
