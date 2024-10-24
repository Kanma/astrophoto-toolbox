# astrophoto-toolbox

A collection of tools related to astrophotography, in a single static library.
Command-line tools are available too. They act as usage examples for the library.

Features:

* RAW files loading and conversion to FITS, or various image file formats
* Registration (= stars detection)
* Plate solving (= finding the celestial coordinates of an image)
* Stacking of several images, which includes:
  * Background calibration
  * Computation of the transformation to apply to one image in order to stack it
    on top of another one
* Detection of the center of the star in a bitmap where a Bahtinov mask is used
* Search in a catalog of Deep-Space Objects for objects matching a pattern


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
| raw2img                   | Convert a RAW image file into a FITS, PPM, PGM, PNG, BMP, JPG, TGA or HDR one             |
| find-coordinates          | Determine the astronomical coordinates of an image                                        |
| register                  | Detect the stars in an image                                                              |
| background-calibration    | Perform background calibration on an image, given a reference image                       |
| compute-transformation    | Compute the translation between two FITS files containing approximately the same stars    |
| stack                     | Perform stacking of several images                                                        |
| search-in-catalog         | Search in the catalog of Deep-Space Objects for objects matching a pattern                |


## Tests

By default, tests are compiled and run alongside the library.

Their compilation can be disabled by setting the ```ASTROPHOTOTOOLBOX_BUILD_TESTS```
option to ```OFF```.

Additionally, the ```ASTROPHOTOTOOLBOX_RUN_TESTS``` option allows to compile the
tests but not run them automatically.


## License

```astrophoto-toolbox``` is licensed under a BSD 3-Clause license.
