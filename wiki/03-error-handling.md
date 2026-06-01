# 03 - Error Handling

## What this page covers

Everything about how the HEIF library reports things to its host, in
all three language tiers (C ABI, HEIFPP wrapper, Java).

The library has **two independent channels** for talking to its
embedder. They share the same shape (install / fallback / silent-by-
default) but carry different kinds of information:

| Channel | Carries | When it fires |
|---|---|---|
| **Error sink** | Structured failure records (code, origin, file:line, message, cause) | A library function fails |
| **Trace sink** | Verbose developer log lines (parser walk, box-walk diagnostics) | A `logInfo()` / `logWarning()` / `logError()` statement runs *and* `Log::setLevel` is above `NONE` |

Both channels are **silent by default** - out-of-the-box, the
library writes nothing to your stdout or stderr. You opt in.

The error channel is documented in three escalating levels:

1. **Lazy** - install one line at startup, get useful diagnostics for
   free
2. **Typical** - install your own sink that routes diagnostics into
   your application's logger
3. **Advanced** - read the diagnostic slot directly for structured
   field-level access

If you just want errors to show up somewhere useful, **the Lazy
section is enough**. The rest is for production embedders. The trace
channel has its own short section near the end of this page.

### Quick: which channel do I want?

```
   Do I want to know when something fails?
      => yes  => install an Error sink (almost everyone wants this)

   Do I want to see verbose developer trace output?
      => only when actively debugging the library itself
      => install a Trace sink AND raise Log::setLevel

   Do I want my application to be silent unless I look?
      => do nothing. That's the default.
```

---

## What you need before starting

- A working consumer project - see
  **[02 - Linking Your Project](02-linking-your-project.md)**.
- Library version v4.0.0 or newer. The slot, sink, and formatter
  helpers in this page were introduced during the v3.8 series and
  are stable across the v4.x ABI promise (see
  [10 - ABI Policy](10-abi-policy.md)).

---

## Mental model: how errors flow

Every public function in the library returns an integer
`HEIF::ErrorCode`. That return value is the *contract*: production
code should always check it.

In addition, when a failure happens, the library writes a structured
record to a thread-local *slot* (`HEIF::ErrorInfo`) describing **what,
where, and why**. That slot is a side-channel - your old code that
only inspects the return value still works.

Finally, if you've installed a *sink* callback, the library calls it
the moment the slot is written. The sink lets you funnel every
failure through one place (your logger, your crash reporter, your
debug overlay) without per-call-site code.

```
   API call returns rc != OK
              │
              ▼
   ┌─────────────────────┐
   │   slot is written   │ ◄── always
   └──────────┬──────────┘
              │
              ▼
   ┌─────────────────────┐
   │  sink fires (if any)│ ◄── opt-in (`setErrorSink`)
   └─────────────────────┘
```

Three things matter:

- **Slot writes are thread-local.** Errors on thread A never leak to
  thread B.
- **Sink dispatch is on the failing thread.** Make sure your
  destination is thread-safe (most loggers are).
- **No sink installed = zero overhead.** The dispatch path is one
  atomic load returning `nullptr` on the success path.

---

## Level 1 - Lazy: get useful diagnostics with one line

Add this anywhere before your first HEIF call (typically top of
`main()`):

```cpp
#include "heiferrorinfo.h"
#include <cstdio>

int main() {
    HEIF::setErrorSink(&HEIF::stderrErrorSink, stderr);
    // ... rest of your application unchanged ...
}
```

`stderrErrorSink` is a reference implementation that ships with the
library; it writes one line per failure to whatever `FILE*` you pass
via the `user` pointer (here, `stderr`).

What you get on every failure, with no other code changes:

```
[heif] [Reader_FileOpen@/path/to/heifreaderimpl.cpp:116] FILE_OPEN_ERROR(8): HeifReaderImpl::initialize: stream is not good (file missing, permission denied, or stream broken)
```

You still check `if (rc != HEIF::ErrorCode::OK)` and branch as
before. The sink just makes sure the verbose breadcrumb shows up
somewhere visible.

---

## Level 2 - Typical: install your own sink

For real applications you usually want diagnostics in your existing
logger, not on stderr. Install a custom sink that does whatever you
need:

```cpp
#include "heiferrorinfo.h"
#include <spdlog/spdlog.h>

void install_heif_diagnostics() {
    HEIF::setErrorSink(
        [](const HEIF::ErrorInfo& info, void* /*user*/) {
            // formatErrorInfo produces the same one-liner stderrErrorSink uses
            spdlog::error("HEIF: {}", HEIF::formatErrorInfo(info));
        },
        nullptr);  // user pointer unused in this lambda
}
```

If your logger needs context (e.g. a session ID), pass it via the
`user` pointer:

```cpp
struct LogContext { MyLogger* logger; std::string session; };
LogContext ctx{&myLogger, sessionId};

HEIF::setErrorSink(
    [](const HEIF::ErrorInfo& info, void* user) {
        auto* c = static_cast<LogContext*>(user);
        c->logger->error("[%s] HEIF %s", c->session.c_str(),
                         HEIF::formatErrorInfo(info).c_str());
    },
    &ctx);  // ctx must outlive any HEIF call
```

The sink signature is a plain C function pointer plus a `void*`, 
chosen for C-ABI compatibility, so pure-C programs and FFI bindings
can install sinks too. Lambdas without captures decay to function
pointers and work directly; lambdas with captures need to round-trip
through the `user` pointer (as above).

To uninstall:

```cpp
HEIF::setErrorSink(nullptr, nullptr);
```

`HEIF::hasErrorSink()` returns `true` if one is currently installed, 
useful for deduplicating if you have both a sink and inline polling.

---

## Level 3 - Advanced: read the slot directly

If you're writing test infrastructure, fuzzers, or wrappers, you may
want the structured fields without going through a string formatter:

```cpp
#include "heiferrorinfo.h"

HEIF::clearErrorInfo();                            // (1) drain stale state
HEIF::ErrorCode rc = reader->initialize(filename); // (2) call

if (rc != HEIF::ErrorCode::OK) {                   // (3) check return
    if (HEIF::hasErrorInfo()) {                    // (4) consult slot
        const HEIF::ErrorInfo& info = HEIF::getErrorInfo();
        myLogger.error_struct({
            .file       = info.file(),         // const char* (__FILE__ literal)
            .line       = info.line(),         // int
            .origin     = info.origin(),       // ErrorOrigin enum
            .code       = info.code(),         // ErrorCode enum (== rc)
            .message    = info.message(),      // const char* (library-owned)
            .cause_what = info.causeWhat(),    // const char* ("" if no exception)
        });
    }
    // (5) branch on the integer code as you always have
    switch (rc) {
        case HEIF::ErrorCode::FILE_OPEN_ERROR:   /* prompt user */ break;
        case HEIF::ErrorCode::FILE_HEADER_ERROR: /* corrupt */ break;
        default: break;
    }
}
```

### Slot contents

`HEIF::ErrorInfo` is a class with `noexcept` accessors (declared in
`srcs/api/common/heiferrorinfo.h`). The accessor names match the
field names in the example above:

| Accessor | Returns | Notes |
|---|---|---|
| `info.code()` | `ErrorCode` | Mirror of the function's return code. |
| `info.origin()` | `ErrorOrigin` | Coarse subsystem (`Reader_FileOpen`, `Writer_BoxBuild`, `Parser_Vvc`, ...). |
| `info.message()` | `const char*` | HEIF-level explanation, set at the failure site. Never null; `""` when no message was attached. Pointer stable until the next mutating call. |
| `info.causeWhat()` | `const char*` | Upstream `std::exception::what()` if the failure was caught from a throw, or the literal `"<unknown exception>"` for `catch(...)` sites. `""` on the pure guard-return path. |
| `info.file()` | `const char*` | `__FILE__` of the failure site (points into static storage). |
| `info.line()` | `int` | `__LINE__` of the failure site. |
| `info.valid()` | `bool` | `false` when the slot is empty. |

The class manages its own heap storage for the message strings, so
the public binary layout contains no `std::string` (which is not
ABI-stable across MSVC CRTs and cannot cross a C FFI boundary).
All accessors are `noexcept`.

### Slot semantics

- **Pure guard returns** (e.g. `UNINITIALIZED`, `INVALID_ITEM_ID`
  from a pre-call sanity check) leave the slot empty.
  `hasErrorInfo() == false` after a non-OK return is itself a
  meaningful signal: *"a guard fired, no exception was caught,
  the integer code is the whole story."*
- **Caught `std::exception`** populates the slot with `causeWhat`
  set to the upstream `what()` string.
- **`catch(...)` sites** populate the slot with
  `causeWhat == "<unknown exception>"` so consumers can
  distinguish guarded returns from eaten unknown exceptions.
- **Successful calls** drain the slot at HEIFPP boundaries, but
  you should not rely on slot state after a successful call, 
  always pair `if (rc != OK)` with the slot read.

### Stable identifier helpers

```cpp
HEIF::errorCodeName(HEIF::ErrorCode::FILE_OPEN_ERROR)
    // => "FILE_OPEN_ERROR"

HEIF::errorOriginName(HEIF::ErrorOrigin::Reader_FileOpen)
    // => "Reader_FileOpen"
```

Always non-null. For values outside the documented enums they
return `"ErrorCode(<n>)"` / `"ErrorOrigin(<n>)"`. Useful for
log lines you want to be diff-friendly across versions.

### Formatter helpers

For when you want the sink's output format but as a string:

```cpp
char buf[256];
std::size_t n = HEIF::formatErrorInfo(info, buf, sizeof(buf));
// snprintf-style: returns chars that would have been written;
// always NUL-terminates when sizeof(buf) > 0.

std::string s = HEIF::formatErrorInfo(info);
// Allocates exactly the required size.
```

Output:
```
[<ORIGIN>@<file>:<line>] <CODE_NAME>(<n>): <message>
[<ORIGIN>@<file>:<line>] <CODE_NAME>(<n>): <message> | cause: <what>
<no error>          ← when info.valid == false
```

---

## HEIFPP (C++ wrapper) tier

The HEIFPP layer mirrors the C ABI's slot + sink + formatter triple,
but uses idiomatic C++ types (`std::function` instead of raw
function pointer, no `void*` user parameter):

```cpp
#include "ErrorContext.h"

// Lambdas with captures work directly:
HEIFPP::setErrorSink([&](const HEIFPP::ErrorContext& ctx) {
    myLogger.error(HEIFPP::formatErrorContext(ctx));
});

// Or the reference stderr sink:
HEIFPP::setErrorSink(&HEIFPP::stderrErrorSink);

// Slot-direct access:
if (HEIFPP::hasLastError()) {
    const HEIFPP::ErrorContext& ctx = HEIFPP::getLastError();
    // ctx.code, ctx.source, ctx.message, ctx.causeWhat,
    // ctx.file, ctx.line, ctx.valid
}

// Uninstall:
HEIFPP::setErrorSink(nullptr);
```

The HEIFPP slot is **independent** from the raw-C++-tier slot. A
HEIFPP sink does not fire on raw-tier `setErrorInfo` calls, and
vice-versa. This is intentional - embedders may want to route the
two tiers to different sinks (e.g. raw tier to syslog, HEIFPP to a
debug overlay).

When a raw-tier error bubbles up through HEIFPP, the raw `ErrorInfo`
is folded into the HEIFPP `ErrorContext.causeWhat` field as a
breadcrumb formatted like
`[<Origin>@<file>:<line>] <message> | cause: <what>` - so HEIFPP
sinks see the full deeper diagnostic too.

---

## Java tier

The Java layer mirrors the same triple, with `Consumer<Exception>`
playing the role of the sink:

```java
import com.nokia.heif.ErrorHandler;
import com.nokia.heif.Exception;

// Anywhere before your first HEIF call:
ErrorHandler.setErrorSink(e -> Log.e("HEIF", ErrorHandler.format(e)));

// Or the reference sink:
ErrorHandler.setErrorSink(ErrorHandler.STDERR_SINK);

// Or any logger:
ErrorHandler.setErrorSink(e -> myLogger.error("HEIF: {}", e));
```

Important properties:

- **Sink fires *before* the Exception is thrown.** Your sink sees
  the same `Exception` instance that's about to unwind the stack, 
  so even if a `try/catch` block forgets to log, your sink already
  did.
- **Misbehaving sinks cannot mask failures.** If your sink throws,
  the dispatcher catches and discards that - the original `Exception`
  still propagates to the caller.
- **The Exception carries native diagnostic fields.** When the
  failure originated in C++, these are populated:

```java
try {
    heif.load("file.heic");
} catch (Exception e) {
    e.getMessage();       // brief HEIF-call-site message
    e.getSource();        // "Reader_FileIO", "Api_Validation", ...
    e.getNativeFile();    // C++ __FILE__ of the failure site
    e.getNativeLine();    // C++ __LINE__
    e.getCauseMessage();  // upstream std::exception::what(), if any
    e.getErrorCode();     // integer code
}
```

See **[04 - Java & Android](04-java-and-android.md)** for full
JNI/Android setup details.

---

## Trace channel (verbose developer log)

Distinct from the error channel above. The trace channel carries the
library's verbose `logInfo()` / `logWarning()` / `logError()` /
`logPanic()` output - parser walk traces, box-walk diagnostics, the
kind of thing you only want to see while actively debugging the
library itself or a fresh embedder integration.

### Defaults

Two gates have to open before a single byte leaves the library on
this channel:

1. `Log::setLevel(...)` must be raised above its default of
   `LogLevel::NONE`, AND
2. either a trace sink is installed, or the byte falls back to
   `std::cout` / `std::cerr`.

So **out of the box, nothing happens.** Existing applications that
have never touched `Log::setLevel` see exactly the same silence they
saw in v3.x.

### Lazy: send trace output to stderr

```cpp
#include "heiferrorinfo.h"
#include "log.hpp"   // for Log::setLevel
#include <cstdio>

Log::setLevel(Log::LogLevel::WARNING);                  // open gate (1)
HEIF::setTraceSink(&HEIF::stderrTraceSink, stderr);     // open gate (2)
```

Each emitted log line now arrives on stderr prefixed with
`[heif:trace] `, one line per callback. Pass `nullptr` for `user`
to fall back to stderr automatically.

### Typical: route into your logger

The sink signature mirrors the error sink - a plain C function
pointer plus a `void*` user, so pure-C and FFI bindings can install
sinks too:

```cpp
using TraceSink = void (*)(const char* line, void* user);
```

Each invocation receives one **complete logical line** - newline
stripped, NUL-terminated - regardless of how the underlying
`operator<<` chain fragmented its writes:

```cpp
HEIF::setTraceSink(
    [](const char* line, void* /*user*/) {
        spdlog::debug("HEIF trace: {}", line);
    },
    nullptr);
Log::setLevel(Log::LogLevel::INFO);   // INFO is the most verbose
```

To uninstall:

```cpp
HEIF::setTraceSink(nullptr, nullptr);
Log::setLevel(Log::LogLevel::NONE);   // (optional) close gate (1) too
```

`HEIF::hasTraceSink()` returns `true` if one is currently installed.

### Advanced: gating by log level

Levels in increasing severity (lowest = most verbose):

| Level | Includes |
|---|---|
| `LogLevel::INFO` | every emitted line |
| `LogLevel::WARNING` | warning + error + panic |
| `LogLevel::ERROR` | error + panic |
| `LogLevel::PANIC` | only panic |
| `LogLevel::NONE` | nothing (the default) |

Setting `Log::setLevel(WARNING)` means a `logInfo()` statement is a
no-op (its bytes never reach the streambuf), but `logWarning()` and
above flow through. This is a coarse filter applied *before* the
streambuf - it costs one comparison on the suppressed path.

### Don't confuse the two channels

| You want… | Use this |
|---|---|
| To know when something **fails** | Error sink (`setErrorSink`) |
| To see the library's **internal trace output** | Trace sink (`setTraceSink`) + `Log::setLevel` |
| Both | Install both. They're independent. |

The error sink fires only when something goes wrong. The trace sink
fires whenever the library has decided to emit a log line - success
paths included. Forwarding trace output into a structured failure
channel will drown your real signals in noise; route them
separately.

### Threading & performance

- Sink dispatch happens on the **producing thread**, like the error
  sink. If your destination logger isn't thread-safe, serialise
  inside your sink callback.
- The per-thread line buffer is `thread_local`, so threads never
  contend with each other for the accumulator.
- **No sink installed + `Log::setLevel == NONE`** (the default): the
  early-out is a single integer comparison; bytes never reach the
  streambuf. Effectively free.
- **No sink installed + level raised:** bytes flow through to
  `std::cout` / `std::cerr` exactly as in v3.x.
- **Sink installed:** one atomic acquire-load per emitted character
  (whether a sink is currently set), then accumulate-and-dispatch on
  newline.

---

## Threading (error channel)

*(Trace-channel threading is covered in the trace-channel section
above.)*

- The slot is `thread_local`. Errors on thread A never leak to
  thread B.
- Sinks are invoked **on whichever thread produced the failure**.
  Make sure your destination logger is thread-safe.
- `setErrorSink` itself is thread-safe to call at any time. Calls
  in flight at install time may observe either the old or the new
  sink, but never a torn pair (the raw C++ tier publishes both
  atomics with release-store ordering; HEIFPP uses a mutex +
  atomic-bool fast path).

For the **full** thread-safety contract across every public
surface (factory, instance, allocator, stream callbacks,
embedder patterns), see **[06 - Threading & Concurrency](06-threading.md)**.

---

## Performance (error channel)

*(Trace-channel performance is covered in the trace-channel section
above.)*

- **No sink installed:** dispatch is one atomic acquire-load that
  returns `nullptr`. Effectively free.
- **Sink installed (raw C++ tier):** two atomic loads + one indirect
  call. No locks.
- **Sink installed (HEIFPP):** atomic-bool flag check (no lock on
  the no-error path). Lock only acquired *inside* `setLastError`,
  which is itself only called on failures.

The success path of any HEIF call has the same cost it had in
v3.7. The sink only adds work when something has already gone
wrong, and "something has gone wrong" is by definition a
non-hot-path event.

---

## Common problems

| Symptom | Likely cause | Fix |
|---|---|---|
| Sink fires but the message is `<no error>` | You called `formatErrorInfo` on a slot that was never written | Always check `info.valid()` before formatting; or wrap your call site with `clearErrorInfo` + check the return code | |
| `info.file()` and `info.line()` look corrupted | `file()` always returns a `__FILE__` literal in static storage, so this should not happen. If you see it, this is a real bug. | Report the issue; copy the string defensively in the meantime |
| Sink fires twice for one error | You both installed a sink AND have inline polling code | Use `HEIF::hasErrorSink()` to suppress one path |
| Java sink sees an Exception with `null` `getNativeFile()` | The failure originated in HEIFPP-only code (no underlying raw-C++-tier failure) | Expected; falls back to the legacy 2-argument throw path. The integer code and message are still valid. |
| Trace sink installed but never fires | `Log::setLevel` is still `NONE` (the default). Both gates have to open. | Call `Log::setLevel(Log::LogLevel::WARNING)` (or `INFO` for very verbose) |
| Trace sink fires from many threads, lines interleave | Sink dispatch is on the producing thread; your callback writes to a non-thread-safe destination | Either serialise inside the sink, or hand the line off to a thread-safe queue |

---

## Where to go next

- **`srcs/examples/example.cpp::example12()`** - runnable demo of
  every slot state on the **raw C++ tier** (guard return, caught
  `std::exception`, `catch(...)`, HEIFPP success/failure boundaries).
  The one-line install pattern itself lives in `example.cpp::main()`
  - a lazy `HEIF::setErrorSink(&HEIF::stderrErrorSink, stderr)` with
  a comment naming common embedder swaps.
- **`srcs/examples/heifpp_example.cpp::example6_errorHandling()`**, 
  same idea on the **HEIFPP tier**: install a `HEIFPP::setErrorSink`
  callback, provoke a failure, read the thread-local slot via
  `HEIFPP::getLastError()`, format with `HEIFPP::formatErrorContext()`.
- **[04 - Java & Android](04-java-and-android.md)** - Java tier
  build and integration details.
- **`srcs/api/common/heiferrorinfo.h`** - full doxygen-grade reference
  for every helper (both error sink and trace sink).

---

## Exception contract (cross-language safety)

The library is designed to embed safely from C, C++, Java/JNI,
Objective-C++ and Windows COM hosts. Public API entry points install
top-level firewalls so that **no C++ exception escapes across a
cross-language boundary** under normal use:

| Layer | Behaviour on internal failure |
|---|---|
| Reader C ABI (`heifreader_c.h`) | Returns `int32_t` error code; populates the per-thread error slot, drained via `heif_get_last_error_*`. Never throws (and could not - `extern "C"`). |
| Raw C++ reader (`heifreader.h`) | Returns `HEIF::ErrorCode`, populates `HEIF::ErrorInfo` slot. Never throws across the public boundary. |
| Raw C++ writer (`heifwriter.h`) | Returns `HEIF::ErrorCode`, populates the slot. Never throws across the public boundary. |
| HEIFPP (`Heif::load`/`save`) | Returns `HEIFPP::Result`, populates `HEIFPP::ErrorContext`. May propagate `std::logic_error` from `HEIF_ASSERT` (programmer-error contract violation). |
| Java JNI shim | Translates any escaping C++ exception to the equivalent Java exception (`IllegalStateException`, `IllegalArgumentException`, `IndexOutOfBoundsException`, `OutOfMemoryError`, `RuntimeException`). Never crashes the JVM. |

If you write directly against HEIFPP, install **one** top-level
`try/catch` in your entry point as a belt-and-braces guard against
contract violations:

```cpp
try {
    heif.load(filename);
    heif.save(outFilename);
} catch (const std::logic_error& e) {
    // HEIF_ASSERT tripped => bug in this code. Inspect e.what() / sink output.
} catch (const std::exception& e) {
    // Unanticipated escape (rare). Library left in defined state.
}
```

A worked example lives in `srcs/examples/example.cpp` (the `main()`
function).

---

## Platform integration checklist

Each host language and platform has a couple of compile / link flags
that must be set correctly for the firewall above to work as
documented. If exceptions are disabled or RTTI is stripped, the
translator can no longer identify the exception type and the JVM /
Obj-C++ runtime / COM host will see `std::terminate` instead of a
clean Java exception.

| Host | Required compile / link settings |
|---|---|
| **Android NDK (JNI)** | Build the JNI shared library with `-fexceptions` (default in `c++_shared`/`c++_static` STL) and **do not strip RTTI** (`-frtti`, also default). Ship the matching `libc++_shared.so` from the NDK so unwinding tables resolve at load time. The translator in `srcs/api-java/cpp/Helpers.cpp` relies on `dynamic_cast` against `std::exception` subclasses - without RTTI every throw maps to plain `RuntimeException`. |
| **iOS / macOS (Obj-C++)** | The `.mm` files that bridge into HEIFPP must be compiled with `-fexceptions` (Xcode default) and `-fobjc-arc-exceptions` if ARC is on. Wrap each public Obj-C method body in a `@try { … } @catch (NSException *e) { … }` and translate `std::exception` => `NSError`/`NSException` analogously to the JNI translator. The library headers themselves are exception-safe and can be included from pure Obj-C++ without any wrapper. |
| **Windows MSVC (DLL)** | Build with `/EHsc` (the default for `cl.exe`). When linking the writer/reader from a different DLL than the one that allocates `Reader*` / `Writer*`, route every cross-DLL deletion through `Reader::Destroy` / `Writer::Destroy` (now `noexcept`) so the allocator that owned the object also releases it. |
| **Windows COM / SEH-only host** | C++ exceptions cannot cross a COM boundary. Build a thin C-style wrapper layer over the C-API (`heifreader.h` / `heifwriter.h`); these already return `ErrorCode` and never throw out, so the wrapper only needs to forward the codes. Do **not** call HEIFPP (`Heif::load` / `Heif::save`) directly from a SEH-only host - it can throw `std::logic_error` from `HEIF_ASSERT`. |
| **Pure C consumers** | Use `heifreader_c.h` + `heif_error_c.h` only - the `extern "C"` ABI facade. Both are fully `try/catch`-firewalled internally; every failure path returns an `int32_t` error code and populates the per-thread error slot (drained via the `heif_get_last_error_*` accessors, or watched via `heif_set_error_sink`). Do **not** include `heifreader.h` / `heifwriter.h` (those are the raw C++ API and require a C++ compiler). The writer C facade is not part of v4.0.0; producing files from pure C is not yet supported. |
| **Server / multi-threaded embedders** | The error slot is **per-thread** (`thread_local`). The sink callback, however, is process-wide and may fire on any thread; if your sink writes to a shared logger, ensure the logger itself is thread-safe. |

If you ship a host application that violates one of these (e.g.
Android with `-fno-exceptions`), the symptom is usually a SIGABRT
inside `__cxa_throw` rather than a Java exception - that's the
runtime aborting because there's no unwinder to reach the JNI
translator. The fix is always at the host build settings, not in this
library.
