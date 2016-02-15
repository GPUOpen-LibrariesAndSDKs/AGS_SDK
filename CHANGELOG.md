# AMD AGS SDK Changelog

See `ags_lib\CHANGELOG.md` for changes to the core AGS library. 

### v3.2.0 - 2016-02-12
* Update ags_sample to use the library version number from the optional info parameter of `agsInit`
* Update crossfire_sample to call `agsInit` prior to device creation
* Update eyefinity_sample with latest DXUT

### v3.1.1 - 2016-01-28
* Display library version number in ags_sample
* Do not display `radeonSoftwareVersion` in ags_sample while we wait for a needed driver update
* Set `WindowsTargetPlatformVersion` to 8.1 for VS2015, to eliminate unnecessary dependency on optional Windows 10 SDK

### v3.1.0 - 2016-01-26
* Initial release on GitHub
