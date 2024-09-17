# astrophoto-toolbox

A collection of tools related to astrophotography, in a single static library.
Command-line tools are available too. They act as usage examples for the library.

Features:

* RAW files loading and conversion to FITS
* Plate solving (= finding the celestial coordinates of an image)
* Computation of the transformation to apply to one image in order to stack it
  on top of another one


## Compilation

```
$ mkdir build
$ cd build
$ cmake ..
```

## Dependencies

CMake is required to compile ```astrophoto-toolbox```.

Other dependencies (*cfitsio*, *astrometry.net*, *libraw*, ...) are automatically
downloaded and compiled alongside the library.


## Command-line tools

The following tools are compiled alongside the library (this can be disabled by setting
the ```ASTROPHOTOTOOLBOX_BUILD_TOOLS``` option to ```OFF```):

| Tool                      | Description                                                                               |
| ------------------------- | ----------------------------------------------------------------------------------------- |
| raw2img                   | Convert a RAW image file into a FITS, PPM or PGM one                                      |
| find-coordinates          | Determine the astronomical coordinates of a FITS or RAW file                              |
| register                  | Detect the stars in a FITS or RAW image                                                   |
| compute-transformation    | Compute the translation between two FITS files containing approximately the same stars    |


## Tests

By default, tests are compiled and run alongside the library.

Their compilation can be disabled by setting the ```ASTROPHOTOTOOLBOX_BUILD_TESTS```
option to ```OFF```.

Additionally, the ```ASTROPHOTOTOOLBOX_RUN_TESTS``` option allows to compile the
tests but not run them automatically.


## License

```astrophoto-toolbox``` is licensed under a BSD 3-Clause license.

**However**, plate-solving is using the *astrometry.net* source code, which altough
being licensed under a BSD 3-Clause license too, requires some GPL dependencies.
Consequently, the whole work must be distributed under the GPL version 3 or later.

Should you not need the plate solving feature, you can disable it by setting the
```ASTROPHOTOTOOLBOX_INCLUDE_PLATESOLVING``` option to ```OFF```, in which case
the GPLv3 doesn't apply and everything remains under the BSD 3-Clause license.
