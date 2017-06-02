### LASAGNE-Core 1.4.4

##### Changes:
- Support for Visual Studio 2017
- Support for GCC 6.2
- Updated supplied SQLite3 package to version 3.19.2
- Added .gitignore file to aid filtering on commits
- Added a version file
- Added a changelog
- Added last_error translation to DAF_OS for Windows error number reporting
- IORBinder used with the Naming Service now defaults to the base context specified via properties
- Changed name of DAF_UNUSED_ARG to DAF_UNUSED_STATIC to better reflect intended use
- Modified training exercises to use assignment to DDS topic instances

##### Fixes:
- Fixed use of magnitude operator for comparisons in DAF_OS
- Fixed enable discovery (within a TAFServer) property to use positive logic
- Fixed support for ARMv7 builds on Debian
- Fixed contribution guidelines to properly refer to develop as the default target for pull requests
- Fixed training IDL filenames to avoid name clashes on RTI builds
- Fixed leftover Subversion tags
- Fixed exception specifications (removed)
- Fixed an issue with CORBAInterfaceHandler servant deactivation through the destructor
- Fixed selective support for OpenDDS Java bindings within projects

##### Notes:
- There is not currently support for wide-character builds
_______________________________________________________________________________

### LASAGNE-Core 1.4.3

##### Notes:
- Initial GitHub release
