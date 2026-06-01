/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#include "customallocator.hpp"

#include <cstdlib>  // std::atexit
#include <new>      // std::bad_alloc -- thrown by customAllocate() on OOM

#include "../api/common/heifallocator.h"

namespace
{
    class DefaultAllocator : public HEIF::CustomAllocator
    {
    public:
        DefaultAllocator()           = default;
        ~DefaultAllocator() override = default;

        void* allocate(size_t n, size_t size) override
        {
            return malloc(n * size);
        }
        void deallocate(void* ptr) override
        {
            free(ptr);
        }
    };
}  // namespace
static DefaultAllocator defaultAllocator;
static HEIF::CustomAllocator* customAllocator;

// True iff `customAllocator` was published by an explicit
// `setCustomAllocator(non-null)` call.  A *lazy* publication of
// `&defaultAllocator` (done by the first `getCustomAllocator()` call below
// when no embedder has installed yet) leaves this flag false, so a
// subsequent embedder install can replace the lazy default without first
// having to call `setCustomAllocator(nullptr)` to reset.
//
// Why this matters (lazy-vs-explicit default-install regression):
// any code path that allocates through the library's `Vector<T>` /
// `String` / `List<T>` typedefs *before* an embedder has installed its
// allocator will trigger the lazy default-install.  For example,
// parameterized registration that runs before `main()` can call
// `Vector<Param>::push_back`.  Pre-fix, that lazy default was
// indistinguishable from an explicit install, so the embedder's
// subsequent `Reader::SetCustomAllocator(...)` would be rejected with
// `ALLOCATOR_ALREADY_SET` and every later allocation would silently
// route through `defaultAllocator` instead of the embedder's tracker.
// Tracking the lazy-vs-explicit distinction here lets the documented
// "first explicit install wins" contract hold even when the lazy
// default has been published in the meantime.
static bool customAllocatorInstalledExplicitly = false;

// Static-destruction-order safety net.
//
// Embedders typically install their custom allocator from a static-storage
// object (e.g. a singleton CountingAllocator wired in by JNI on Android,
// or a function-local static in embedder code). The user object's
// lifetime is therefore controlled by THEIR translation unit's static
// destruction order, which is unrelated to ours.
//
// If the user's allocator is destroyed before the library's last static
// (any global-or-function-local Vector, std::string in a logger, etc.),
// any deallocation routed through `customAllocator` would dispatch on a
// dead vtable and hit a pure-virtual stub - process aborts with
// "pure virtual method called" right at the end of the run.
//
// Mitigation: the FIRST time a non-null custom allocator is installed,
// register an atexit handler that resets `customAllocator` back to the
// always-alive `defaultAllocator`. Because atexit handlers and static
// destructors are both managed by __cxa_atexit and run in reverse
// registration order, registering at install-time guarantees the reset
// fires BEFORE the user's later-constructed allocator object is
// destroyed. Any allocation/deallocation that then runs during library
// shutdown lands on `defaultAllocator`, which lives until program exit.
static void resetCustomAllocatorAtExit()
{
    customAllocator = &defaultAllocator;
    // Treat the safety-net reset as a *lazy* publication, not an
    // explicit user install: if the embedder were to call
    // setCustomAllocator(...) again during shutdown (rare but legal in
    // long-lived processes that re-init the library), they should not
    // be blocked by ALLOCATOR_ALREADY_SET on a slot we wrote here.
    customAllocatorInstalledExplicitly = false;
}

bool setCustomAllocator(HEIF::CustomAllocator* customAllocator_)
{
    // Acceptance rules:
    //   * setCustomAllocator(nullptr) -- the documented reset path --
    //     always succeeds and clears the explicit-install flag so the
    //     next install starts from a clean slate.
    //   * setCustomAllocator(non-null) succeeds when the slot is
    //     either truly empty (`customAllocator == nullptr`) OR
    //     currently holding the *lazy* `defaultAllocator` published
    //     by `getCustomAllocator()` before any explicit install.  The
    //     lazy default is, by definition, "no embedder has chosen
    //     yet", so the first explicit install is allowed to replace
    //     it (lazy-vs-explicit fix -- see `customAllocatorInstalledExplicitly`
    //     comment above).
    //   * Otherwise -- an explicit non-null install is already in
    //     place -- return false so `Reader::SetCustomAllocator()`
    //     surfaces `ALLOCATOR_ALREADY_SET` to the caller, matching
    //     the long-standing "single explicit install per process
    //     unless reset via nullptr first" header contract.
    if (customAllocator_ == nullptr)
    {
        customAllocator                    = nullptr;
        customAllocatorInstalledExplicitly = false;
        return true;
    }

    if ((customAllocator == nullptr) || !customAllocatorInstalledExplicitly)
    {
        customAllocator                    = customAllocator_;
        customAllocatorInstalledExplicitly = true;
        // One-shot registration. atexit fires the handler in reverse
        // registration order, so a single registration suffices for
        // every subsequent install / nullptr-reset / re-install cycle.
        static bool atexitRegistered = false;
        if (!atexitRegistered)
        {
            std::atexit(&resetCustomAllocatorAtExit);
            atexitRegistered = true;
        }
        return true;
    }

    return false;
}

HEIF::CustomAllocator* getDefaultAllocator()
{
    return &defaultAllocator;
}

HEIF::CustomAllocator* getCustomAllocator()
{
    if (!customAllocator)
    {
        // Lazy publication of the always-alive default allocator so
        // any allocation that happens before an embedder installs
        // (e.g. parameterized-test registration that runs before
        // `main()`) is still serviced.  Crucially, this lazy install
        // does NOT set `customAllocatorInstalledExplicitly`, so a
        // subsequent embedder `setCustomAllocator(...)` is allowed
        // to replace this default -- see lazy-vs-explicit fix above.
        customAllocator = getDefaultAllocator();
    }
    return customAllocator;
}

void* customAllocate(size_t size)
{
    // OOM-safe allocation point (S.10 / R.x). The CustomAllocator contract
    // (heifallocator.h) lets allocate() return nullptr on bad_alloc-style
    // pressure. Every internal call site routes through here via either
    // CUSTOM_NEW (placement-new on the returned pointer) or
    // customAllocateArray (writes a header at the returned pointer); both
    // would deref a null buffer and SIGSEGV. Translate the contract to
    // std::bad_alloc here so the public-API HEIF_ENTRY_TRY /
    // HEIF_CATCH_AND_RETURN firewall converts it to a non-OK return code
    // for the embedder, instead of crashing the host process.
    void* p = getCustomAllocator()->allocate(size, 1);
    if (p == nullptr)
    {
        throw std::bad_alloc();
    }
    return p;
}

void customDeallocate(void* ptr)
{
    getCustomAllocator()->deallocate(ptr);
}
