/*
 * ELF version script for libheif_shared.so.4 / libheif_writer_shared.so.4.
 *
 * Belt-and-braces enforcement of the symbol visibility audit.
 *
 * Even with `-fvisibility=hidden -fvisibility-inlines-hidden` set in
 * srcs/CMakeLists.txt, a handful of accidental leaks survive -- chiefly
 * `std::` template instantiations that get implicit default visibility
 * because they are referenced from inline accessors of public classes.
 * The version script makes the *intent* explicit at link time:
 *
 *   - Public:  the HEIF:: and HEIFPP:: C++ namespaces, plus the
 *              extern "C" surface (`heif_*`).  Their typeinfo /
 *              vtable / typeinfo-name records are also exported
 *              because public polymorphic types need them across
 *              the .so boundary for `dynamic_cast` / `typeid` / RTTI.
 *
 *   - Private: everything else.  In particular:
 *                * `std::` instantiations
 *                * anonymous-namespace symbols
 *                * any third-party object dependencies (vvcparser
 *                  internals are statically embedded but must not
 *                  surface as public ABI).
 *
 * Versioning policy:
 *   - The single version tag `HEIF_4.0` covers all symbols introduced
 *     up to and including the v4.0 line.  Future v4.x patch releases
 *     append symbols under the same tag (additive changes only).
 *   - A v5.0 SOVERSION bump introduces a new `HEIF_5.0` tag and may
 *     remove or re-tag symbols.
 *
 * Reading hints:
 *   - `extern "C++" { "HEIF::*" }` matches mangled C++ symbols by
 *     their demangled prefix.  The matcher is glob-style (* and ?),
 *     not regex.  This is the GNU ld extension; LLD also accepts it.
 *   - `local: *;` at the bottom is the catch-all that turns this
 *     allowlist into a hard allowlist instead of a hint.
 */

HEIF_4.0 {
    global:
        /* Pure-C reader and error surface (heif_reader_*,
         * heif_get_last_error_*, heif_clear_last_error,
         * heif_has_last_error, etc.).  All `heif_*` C entry points are
         * intentionally part of the v4.0 ABI; new entry points appended
         * to this prefix in v4.x patches inherit the same tag. */
        heif_*;

        /* C++ public surface: HEIF:: and HEIFPP:: namespaces.  We match
         * by the Itanium-ABI mangled prefix so the rule is robust against
         * any glob-on-demangled-name quirks in the host linker:
         *
         *     _ZN4HEIF*    ->  HEIF::*           (non-const member fns,
         *                                         free fns, ctors)
         *     _ZNK4HEIF*   ->  HEIF::* (const)   (const member fns)
         *     _ZN6HEIFPP*  ->  HEIFPP::*
         *     _ZNK6HEIFPP* ->  HEIFPP::* (const)
         *     _ZTV*        ->  vtable for *
         *     _ZTI*        ->  typeinfo for *
         *     _ZTS*        ->  typeinfo name for *
         *     _ZTT*        ->  VTT for *
         *     _ZTC*        ->  construction vtable for *
         *
         * The vtable / typeinfo / typeinfo-name rules are scoped to
         * HEIF / HEIFPP only; std:: typeinfos must stay hidden so we
         * do not accidentally start exporting std::vector RTTI.
         */
        _ZN4HEIF*;
        _ZNK4HEIF*;
        _ZN6HEIFPP*;
        _ZNK6HEIFPP*;
        _ZTVN4HEIF*;
        _ZTVN6HEIFPP*;
        _ZTIN4HEIF*;
        _ZTIN6HEIFPP*;
        _ZTSN4HEIF*;
        _ZTSN6HEIFPP*;
        _ZTTN4HEIF*;
        _ZTTN6HEIFPP*;
        _ZTCN4HEIF*;
        _ZTCN6HEIFPP*;

    local:
        /* Hard catch-all: everything not explicitly listed above is
         * hidden.  std::* instantiations, anonymous-namespace helpers,
         * vvcparser internals, etc. */
        *;
};
