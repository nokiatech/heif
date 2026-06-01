# 04 - Java & Android

## What this page covers

How to build and use the Java bindings (desktop JVM) and Android
bindings (AAR). Covers what the JNI bridge gives you, where the
artefacts land in your build tree, and the Android-specific patterns
for error handling.

---

## What you need before starting

Desktop:

- A working C++ build of the library (page 01)
- **JDK 8 or newer**
- **Gradle 6 or newer**

Android:

- A working C++ build of the library (page 01)
- **Android SDK** (matching `compileSdkVersion` in the Gradle file)
- **Android NDK** with `ANDROID_NDK` environment variable set
- **Ninja** (Windows build script requires it; Linux is optional)

You do **not** need Android Studio - the build is fully scripted, 
but you can use it if you prefer.

---

## What's in the Java tier

Three layers, top down:

| Layer | Source | What it does |
|---|---|---|
| Public Java API | `srcs/api-java/java/com/nokia/heif/` | Idiomatic Java classes (`Reader`, `Writer`, `Image`, `Exception`, `ErrorHandler`, ...) |
| JNI bridge (C++) | `srcs/api-java/cpp/` | Translates between the JVM and the HEIFPP layer; produces `libheifjni.so` / `heifjni.dll` |
| HEIFPP wrapper | `srcs/api-cpp/` | The C++ library that actually does the work |

The Java API is **complete enough to build a real HEIF viewer or
encoder app** - every reader and writer feature exposed by the C++
API has a Java equivalent.

---

## Walkthrough - Desktop JVM

### Step 1: Build the C++ side

From the repository root:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

This produces `libheifjni.so` (Linux), `libheifjni.dylib` (macOS),
or `heifjni.dll` (Windows) under `build/lib/`.

### Step 2: Build the Java JAR

```bash
cd build/java-desktop
gradle build
```

The JAR ends up at `build/java-desktop/build/libs/heif-<version>.jar`.

### Step 3: Run a Java program against it

You need both the JAR (for compile and runtime) and the JNI shared
library (for runtime, on `java.library.path`):

```bash
javac -cp build/java-desktop/build/libs/heif-4.0.0.jar MyApp.java

java -cp build/java-desktop/build/libs/heif-4.0.0.jar:. \
     -Djava.library.path=build/lib \
     MyApp
```

Or, if you bundle your own JAR with Maven/Gradle, declare the HEIF
JAR as an `implementation` dependency and ship the appropriate native
`.so`/`.dll`/`.dylib` for each target platform.

### Minimal Java application

```java
// MyApp.java
import com.nokia.heif.HEIF;
import com.nokia.heif.ErrorHandler;
import com.nokia.heif.Exception;

public class MyApp {
    public static void main(String[] args) {
        // Recommended for new code: route every native diagnostic to stderr.
        // (Replace with your logger of choice for production.)
        ErrorHandler.setErrorSink(ErrorHandler.STDERR_SINK);

        if (args.length < 1) {
            System.err.println("usage: MyApp <file.heic>");
            System.exit(1);
        }

        HEIF heif = new HEIF();
        try {
            heif.load(args[0]);
            System.out.println("OK: loaded " + heif.getImages().size() + " image(s)");
        } catch (Exception e) {
            // The sink already printed the verbose diagnostic.
            // Decide what to do based on the integer code:
            System.err.println("Failed: code=" + e.getErrorCode());
            System.exit(2);
        } finally {
            heif.release();
        }
    }
}
```

---

## Walkthrough - Android

### Step 1: Set the NDK path

```bash
export ANDROID_NDK=/path/to/android-ndk-r25
```

(Adjust to your installation. The Android Gradle plugin reads this
variable - or `ANDROID_HOME` plus a discoverable `ndk/<version>/`
subdirectory - to locate the NDK toolchain.)

### Step 2: Build the AAR

The Android side is a standard Android Gradle project. From the
repository root:

```bash
cd build/android
gradle assembleRelease
```

This cross-compiles the native library for the ABIs configured in
`build/android/build.gradle` (today: `armeabi-v7a`, `arm64-v8a`,
`x86` - edit `chosenAbiFilters` to add `x86_64` for a 64-bit
emulator), then assembles the AAR. Use `gradle assembleDebug` for
a debug variant.

### Step 3: Locate the AAR

The AAR lands at:

```
build/android/api-java/build/outputs/aar/api-java-release.aar
```

### Step 4: Use the AAR in your Android app

In your app's `build.gradle`:

```gradle
dependencies {
    implementation files('libs/api-java-release.aar')
}
```

(After copying the AAR into your app's `libs/` directory.)

For published distribution, deploy the AAR to a Maven repository
your app reads from. See **`build/android/build.gradle`** for the
Maven publish configuration the project itself uses.

### Step 5: Recommended Android startup pattern

```java
// In your Application.onCreate, or any activity before first HEIF use:
import android.util.Log;
import com.nokia.heif.ErrorHandler;

ErrorHandler.setErrorSink(e ->
    Log.e("HEIF", ErrorHandler.format(e))
);
```

Now every native HEIF failure shows up in `logcat` under the `HEIF`
tag, with full origin / file / line / cause information - without
modifying any of your existing `try/catch` blocks.

If you use Crashlytics, Sentry, or similar:

```java
ErrorHandler.setErrorSink(e -> {
    Log.e("HEIF", ErrorHandler.format(e));
    FirebaseCrashlytics.getInstance().recordException(e);
});
```

---

## What's in `com.nokia.heif.Exception`

The Exception class carries six diagnostic accessors:

| Method | Returns | Description |
|---|---|---|
| `getErrorCode()` | `int` | Integer code matching the C++ tier's `HEIF::ErrorCode` enum |
| `getMessage()` | `String` | Brief HEIF-call-site message (inherited from `java.lang.Throwable`) |
| `getSource()` | `String` | Coarse subsystem name (e.g. `"Reader_FileIO"`, `"Api_Validation"`); the constant `Exception.SOURCE_UNKNOWN` when no origin was recorded |
| `getNativeFile()` | `String` | C++ `__FILE__` of the failure site; `""` if not recorded |
| `getNativeLine()` | `int` | C++ `__LINE__` of the failure site; `0` if not recorded |
| `getCauseMessage()` | `String` | Upstream `std::exception::what()`, or `""` if none |

The `getNativeFile`/`getNativeLine` accessors are the ones that
make crash reports actionable - they pinpoint the exact C++ line
in the library that gave up.

If a failure originated in HEIFPP-only code with no underlying
native diagnostic, `getNativeFile()` returns `""` and
`getNativeLine()` returns `0`. The integer code and message are
always valid.

---

## What `ErrorHandler.format(Exception)` produces

A single human-readable line, identical in shape to what the C++
sinks produce, so your log lines look the same regardless of which
tier they come from:

```
[heif] code=FILE_OPEN_ERROR(8) | source=Reader_FileIO | at /path/to/heifreaderimpl.cpp:116 | cause=std::runtime_error: ... | info=HeifReaderImpl::initialize: stream is not good
```

For programmatic access to the underlying fields, ignore `format()`
and read the accessors directly.

---

## Threading

- The native slot is `thread_local` - Java threads each see their
  own native-tier diagnostic.
- Sinks fire **on whichever thread the failure happened on**, 
  including background workers. Make sure your sink is thread-safe
  (`Log.e` is, most logger libraries are).
- On Android, you cannot touch UI from a sink that fires on a
  background thread. If you want to show a `Toast` or update a
  `TextView`, post a `Runnable` to the main thread:

```java
ErrorHandler.setErrorSink(e -> {
    Log.e("HEIF", ErrorHandler.format(e));
    new Handler(Looper.getMainLooper()).post(() ->
        Toast.makeText(context, "HEIF failure: " + e.getMessage(),
                       Toast.LENGTH_SHORT).show()
    );
});
```

---

## Common problems

| Symptom | Likely cause | Fix |
|---|---|---|
| `UnsatisfiedLinkError: no heifjni in java.library.path` | JNI library not on the runtime path | Pass `-Djava.library.path=<dir-containing-libheifjni.so>` to `java`, or set `LD_LIBRARY_PATH` |
| `cannot find symbol: ErrorHandler.setErrorSink` | JAR was built against pre-v4 sources without the error-sink hook | Rebuild against current sources; check `BuildInfo.Version` |
| Exception thrown but `getNativeFile()` returns `""` | Failure originated in HEIFPP-only code with no underlying native diagnostic | Expected; legacy 2-arg throw path. Integer code + message still valid |
| Android NDK build fails: `Could not find ANDROID_NDK` | Env var not set or pointing to wrong dir | `export ANDROID_NDK=/path/to/ndk` and verify `$ANDROID_NDK/build/cmake/android.toolchain.cmake` exists |
| AAR builds but app crashes at runtime with `dlopen failed` | Native libs not packaged for the device's ABI | Check the `jniLibs/<abi>/` folders inside the AAR; rebuild for missing ABIs |
| Sink fires once for a HEIF call but the Exception is *also* thrown | This is the intended behaviour - the sink is an observer, not a replacement for the throw | Either remove your `try/catch` logging (the sink covers it) or check `e == sinkSeen` to dedupe |

---

## Where to go next

- **`srcs/api-java/java/com/nokia/heif/`** - full Java API surface.
  Each class has Javadoc with usage examples.
- **[03 - Error Handling](03-error-handling.md)** - the C++/HEIFPP
  side of the same error story; useful background for understanding
  what flows up over JNI.
- **`build/android/api-java/build.gradle`** - reference Gradle setup
  for an Android consumer of this library.
