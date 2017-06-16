# Paneadra Python Bridge

The Paneadra Python Bridge enables you to embed the CPython interpreter into a runtime that supports dynamic loading of Unix _shared objects_ and/or Windows _DLLs_.

It was written for embedding CPython into the Progress OpenEdge ABL runtime, to facilitate software integration with various third party projects.

The Paneadra Python Bridge is written in C. Supported platforms are currently: Windows, Linux, IBM AIX.

# Build

## Gcc compile on Linux

As an example, here is the runbook for compiling on a standard Ubuntu Xenial x64 distibution, running on Vagrant plus VirtualBox on a Windows workstation.

1. Create a `panaedra` folder somewhere on your windows machine
1. Add an environment variable `PanaedraWorkspace` that points to the folder above. End with an extra backslash.
    * _Example:_ `PanaedraWorkspace=C:\work\panaedra\`
1. Pull the `panaedra_c_pythonbridge` repository into the `%PanaedraWorkspace%` folder
1. Install Vagrant and VirtualBox, make sure VTX is on in your BIOS, and setup a vagrant machine of Ubuntu Xenial x64, latest version
1. Go to the vagrant vm folder, and halt the vm:
    ```bat
    cd /d "C:\vm\ubuntu.xenial64"
    vagrant halt
    ```
1. Put the following in the `Vagrantfile`:
    ```bash
    config.vm.synced_folder "../../work/panaedra", "/panaedra"
    ```
1. Go to the vagrant vm folder, and up the vm, plus putty:
    ```bat
    cd /d "C:\vm\ubuntu.xenial64"
    vagrant up
    vagrant putty
    ```
1. From the ssh terminal, install prerequisites and create the shared object:
    ```bash
    sudo apt install python 2.7-dev
    sudo apt install python-dev
    cd "/panaedra/panaedra_c_pythonbridge/src/panaedra/mxroot/mxpython/c_source/"
    # Compiling
    gcc $(python2.7-config --cflags) -fPIC -c panaedra_pythonbridge.c
    # Linking, create shared object.
    # Note: Do an explicit link to libpython as well, because OE does not allow undefined symbols (even when they are fine at runtime).
    # Symptoms:
    #   DLL Error : /panaedra/panaedra_c_pythonbridge/src/panaedra/mxroot/mxpython/c_source/panaedra_pythonbridge.so
    #   undefined symbol: _PyByteArray_empty_string (8014)
    #   Could not load DLL procedure panaedra/mxroot/mxpython/c_source/panaedra_pythonbridge.so. (3258)
    # Ref: https://github.com/Cantera/cantera/issues/319 , https://stackoverflow.com/questions/16112310/explicitly-linking-a-local-shared-object-library , 
    gcc $(python2.7-config --cflags --ldflags) -shared -o panaedra_pythonbridge.so panaedra_pythonbridge.o -lc /usr/lib/x86_64-linux-gnu/libpython2.7.so.1.0
    # Intermediate object file can be removed
    rm panaedra_pythonbridge.o
    # Do a quick check for undefined symbols (see above)
    ldd -r panaedra_pythonbridge.so

    ```
1. If you are a Progress OpenEdge user, now's a good time to look at the `panaedra_oe_demo_lowdep` repository, to see the Python Bridge in action.
