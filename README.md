WinServiceRunner
================

**WinServiceRunner** is a C++ library built in Microsoft Visual Studio 2013 the main goal of which is to work with existing or prepared (**exe** file) Windows Services. The project generates **DLL** which can be used on Windows platform.

What the library **can do**:

 * Install/uninstall a service written either in C++ or in C#
 * Return given service state
 * Start/stop/pause/resume a given service

What the library **can not do** (the list of TODOs):

 * Work with service' dependencies
 * Work with service arguments

**Project settings**

* Preprocessor directive **LIBRARY_API**  
 
Explicitly defined in: 
>"Project Settings" -> "C/C++" -> "Preprocessor" -> "Preprocessor definitions".
   
If defined, equal to **\_\_declspec(dllexport)**, otherwise to **\_\_declspec(dllimport)**.

* Preprocessor directive **DLL_IMPORT**

Makes sense only when **LIBRARY_API** is not defined as it serves only to decorate the functions meant to be internal.

* Preprocessor directive **STRING_ENCODING**

Is an alias for either **char** or **wchar_t** depending on **UNICODE** definition presence.

**Elevating privileges**

Some services need to be handled under Administrator account or with Administrative rights. 

This can be done via spawning a separate process programmatically either in C++ or in C#. Another way is to explicitly set option **UAC Execution Level** to:

> **requireAdministrator (/level='requireAdministrator')**: 

The settings can be found in:

> "Configuration Properties" -> "Linker" -> "Manifest File"
