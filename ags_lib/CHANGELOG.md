# AMD AGS Library Changelog

### v3.2.0 - 2016-02-12
* Add ability to disable Crossfire
  * This is in addition to the existing ability to enable the explicit Crossfire API
  * Desired Crossfire mode is now passed in to `agsInit`
  * Separate `SetCrossfireMode` function has been removed from the AGS API
  * The `agsInit` function should now be called **prior to device creation**
* Return library version number in the optional info parameter of `agsInit`
* Build amd_ags DLLs such that they do not depend on any Microsoft Visual C++ redistributable packages

### v3.1.1 - 2016-01-28
* Return null for the context when initialization fails
* Add version number defines to `amd_ags.h`
* Remove `radeonSoftwareVersion` until needed driver update is public

### v3.1.0 - 2016-01-26
* Initial release on GitHub
