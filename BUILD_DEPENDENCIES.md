# Build Dependencies for PWDownloader

## Available SDKs and Libraries in PWClient-152

### ✅ Angelica Framework SDK (Engine/Output/)

#### 1. **afsdk** (Angelica File SDK) - ⭐ CRITICAL for PWDownloader
**Location:** `PWClient152/Engine/Output/afsdk/`

**Include Files:**
- `AFilePackage.h` - PCK file operations
- `AFilePackMan.h` - PCK manager (g_AFilePackMan)
- `AFilePackBase.h` - Base PCK operations
- `AFilePackGame.h` - Game-specific PCK
- `AFile.h` - File I/O
- `AFileImage.h` - Image file operations
- `AIniFile.h` - INI file parser
- `ATextTabFile.h` - Tab-separated text files

**Libraries:**
- `AngelicaCommon_d.lib` (1.1 MB) - Debug
- `AngelicaFile_d.lib` (1.7 MB) - Debug

**Note:** Only debug libraries available. Release libraries might be in another location.

---

#### 2. **acsdk** (Angelica Common SDK)
**Location:** `PWClient152/Engine/Output/acsdk/`

**Include Files:** (check needed)
**Libraries:** (check needed)

---

#### 3. **amsdk** (Angelica Math SDK?)
**Location:** `PWClient152/Engine/Output/amsdk/`

**Include Files:** (check needed)
**Libraries:** (check needed)

---

#### 4. **a3dsdk** (3D SDK?)
**Location:** `PWClient152/Engine/Output/a3dsdk/`

**Include Files:** (check needed)

---

### ✅ Libraries (Libraries/)

**Location:** `PWClient152/Libraries/`

#### **Include:**
- `zlib/` - ZLIB compression headers
  - `zlib.h`
  - `zconf.h`
  - `deflate.h`
  - etc.

#### **Lib:**
- `Async_d.lib`
- `CxImage_d.lib` (image processing)
- `jpeg_d.lib` (JPEG support)
- `lua5.1_d.lib` (Lua scripting)
- `mpg123lib_d.lib` (MP3 decoding)
- `ogg_d.lib`, `vorbis_d.lib` (OGG Vorbis audio)
- `png_d.lib` (PNG images)
- `strmbase_d.lib` (DirectShow base)
- `tiff_d.lib` (TIFF images)

**Note:** All libraries are DEBUG versions (_d suffix)

---

### ✅ Angelica2.2

**Location:** `PWClient152/Angelica2.2/output/`

#### **3rdSDK**
**Include:** (check needed)

#### **aui**
**Include:** (check needed)
**Lib:** (check needed)

---

## Build Configuration for VS2022

### Include Paths (relative to PWClient152/ClientTools/PWDownloader/):

```
../../Engine/Output/afsdk/include
../../Engine/Output/acsdk/include
../../Engine/Output/amsdk/include
../../Engine/Output/a3dsdk/include
../../../Libraries/include
../../../Angelica2.2/output/3rdSDK/include
../../../Angelica2.2/output/aui/include
../../../../SDK/3rdSDK/include  (from .dsp file)
../../../../SDK/acsdk/Include   (from .dsp file)
```

### Library Paths:

```
../../Engine/Output/afsdk/lib
../../../Libraries/lib
../../../Angelica2.2/output/aui/lib
../../../../CNewSkin/lib  (from .dsp file)
```

### Required Libraries to Link:

**Debug Configuration:**
- `AngelicaCommon_d.lib`
- `AngelicaFile_d.lib`
- Plus other MFC and Windows libs (gdiplus.lib, etc.)

**Release Configuration:**
- Need to find release versions (without _d suffix)

---

## Current Issues

### ⚠️ Missing Release Libraries
All available libraries are DEBUG versions (_d suffix).
Release builds might fail or need different library paths.

### ⚠️ CNewSkin Dependencies
Paths reference `../../../CNewSkin/` which might not exist or be in different location.

---

## Recommendation

**USE REAL SDK instead of stub headers!**

The actual Angelica Framework SDK IS AVAILABLE in this repository:
- Headers: `PWClient152/Engine/Output/afsdk/include/`
- Libraries: `PWClient152/Engine/Output/afsdk/lib/`

Update PWInstallDlg.cpp to use real SDK:
```cpp
#include <AFilePackage.h>  // Real file from afsdk/include
#include <AFilePackMan.h>  // Real file from afsdk/include
```

And update .vcxproj include paths to point to `../../Engine/Output/afsdk/include`
