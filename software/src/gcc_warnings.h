#if defined(__clang__)
#pragma clang diagnostic error "-Weverything"

// We use at least C++20
#pragma clang diagnostic ignored "-Wc++98-compat"
#pragma clang diagnostic ignored "-Wpre-c++14-compat"
#pragma clang diagnostic ignored "-Wpre-c++14-compat-pedantic"
#pragma clang diagnostic ignored "-Wpre-c++17-compat"
#pragma clang diagnostic ignored "-Wpre-c++17-compat-pedantic"
#pragma clang diagnostic ignored "-Wpre-c++20-compat"
#pragma clang diagnostic ignored "-Wpre-c++20-compat-pedantic"

// https://clang.llvm.org/docs/SafeBuffers.html That's too much for now
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"

// We have to add the switch default to satisfy GCC.
#pragma clang diagnostic ignored "-Wcovered-switch-default"
#endif

// Use check_gcc_warnings.py when updating the compiler to find new warnings to enable.

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic error "-Wall"
#pragma GCC diagnostic error "-Wextra"

// Should be enabled by default, but it doesn't hurt to be explicit
#pragma GCC diagnostic error "-Wcast-user-defined"
#pragma GCC diagnostic error "-Wconversion-null"
#pragma GCC diagnostic error "-Wexceptions"
#pragma GCC diagnostic error "-Wglobal-module"
#pragma GCC diagnostic error "-Winherited-variadic-ctor"
#pragma GCC diagnostic error "-Winvalid-offsetof"
#pragma GCC diagnostic error "-Wliteral-suffix"
#pragma GCC diagnostic error "-Wmissing-requires"
#pragma GCC diagnostic error "-Wmissing-template-keyword"
#pragma GCC diagnostic error "-Wpacked-bitfield-compat"
#pragma GCC diagnostic error "-Wpmf-conversions"
#pragma GCC diagnostic error "-Wsubobject-linkage"
#pragma GCC diagnostic error "-Wvexing-parse"
#pragma GCC diagnostic error "-WNSObject-attribute"
#pragma GCC diagnostic error "-Waddress-of-packed-member"
#pragma GCC diagnostic error "-Waggressive-loop-optimizations"
#pragma GCC diagnostic error "-Wanalyzer-allocation-size"
#pragma GCC diagnostic error "-Wanalyzer-deref-before-check"
#pragma GCC diagnostic error "-Wanalyzer-double-fclose"
#pragma GCC diagnostic error "-Wanalyzer-double-free"
#pragma GCC diagnostic error "-Wanalyzer-exposure-through-output-file"
#pragma GCC diagnostic error "-Wanalyzer-exposure-through-uninit-copy"
#pragma GCC diagnostic error "-Wanalyzer-fd-access-mode-mismatch"
#pragma GCC diagnostic error "-Wanalyzer-fd-double-close"
#pragma GCC diagnostic error "-Wanalyzer-fd-leak"
#pragma GCC diagnostic error "-Wanalyzer-fd-phase-mismatch"
#pragma GCC diagnostic error "-Wanalyzer-fd-type-mismatch"
#pragma GCC diagnostic error "-Wanalyzer-fd-use-after-close"
#pragma GCC diagnostic error "-Wanalyzer-fd-use-without-check"
#pragma GCC diagnostic error "-Wanalyzer-file-leak"
#pragma GCC diagnostic error "-Wanalyzer-free-of-non-heap"
#pragma GCC diagnostic error "-Wanalyzer-imprecise-fp-arithmetic"
#pragma GCC diagnostic error "-Wanalyzer-infinite-loop"
#pragma GCC diagnostic error "-Wanalyzer-infinite-recursion"
#pragma GCC diagnostic error "-Wanalyzer-jump-through-null"
#pragma GCC diagnostic error "-Wanalyzer-malloc-leak"
#pragma GCC diagnostic error "-Wanalyzer-mismatching-deallocation"
#pragma GCC diagnostic error "-Wanalyzer-null-argument"
#pragma GCC diagnostic error "-Wanalyzer-null-dereference"
#pragma GCC diagnostic error "-Wanalyzer-out-of-bounds"
#pragma GCC diagnostic error "-Wanalyzer-overlapping-buffers"
#pragma GCC diagnostic error "-Wanalyzer-possible-null-argument"
#pragma GCC diagnostic error "-Wanalyzer-possible-null-dereference"
#pragma GCC diagnostic error "-Wanalyzer-putenv-of-auto-var"
#pragma GCC diagnostic error "-Wanalyzer-shift-count-negative"
#pragma GCC diagnostic error "-Wanalyzer-shift-count-overflow"
#pragma GCC diagnostic error "-Wanalyzer-stale-setjmp-buffer"
#pragma GCC diagnostic error "-Wanalyzer-tainted-allocation-size"
#pragma GCC diagnostic error "-Wanalyzer-tainted-array-index"
#pragma GCC diagnostic error "-Wanalyzer-tainted-assertion"
#pragma GCC diagnostic error "-Wanalyzer-tainted-divisor"
#pragma GCC diagnostic error "-Wanalyzer-tainted-offset"
#pragma GCC diagnostic error "-Wanalyzer-tainted-size"
#pragma GCC diagnostic error "-Wanalyzer-undefined-behavior-strtok"
#pragma GCC diagnostic error "-Wanalyzer-unsafe-call-within-signal-handler"
#pragma GCC diagnostic error "-Wanalyzer-use-after-free"
#pragma GCC diagnostic error "-Wanalyzer-use-of-pointer-in-stale-stack-frame"
#pragma GCC diagnostic error "-Wanalyzer-use-of-uninitialized-value"
#pragma GCC diagnostic error "-Wanalyzer-va-arg-type-mismatch"
#pragma GCC diagnostic error "-Wanalyzer-va-list-exhausted"
#pragma GCC diagnostic error "-Wanalyzer-va-list-leak"
#pragma GCC diagnostic error "-Wanalyzer-va-list-use-after-va-end"
#pragma GCC diagnostic error "-Wanalyzer-write-to-const"
#pragma GCC diagnostic error "-Wanalyzer-write-to-string-literal"
#pragma GCC diagnostic error "-Wattribute-warning"
#pragma GCC diagnostic error "-Wattributes"
#pragma GCC diagnostic error "-Wbuiltin-declaration-mismatch"
#pragma GCC diagnostic error "-Wbuiltin-macro-redefined"
#pragma GCC diagnostic error "-Wcannot-profile"
#pragma GCC diagnostic error "-Wcomplain-wrong-lang"
#pragma GCC diagnostic error "-Wcoverage-invalid-line-number"
#pragma GCC diagnostic error "-Wcoverage-mismatch"
#pragma GCC diagnostic error "-Wcoverage-too-many-conditions"
#pragma GCC diagnostic error "-Wcpp"
#pragma GCC diagnostic error "-Wdeprecated"
#pragma GCC diagnostic error "-Wdeprecated-declarations"
#pragma GCC diagnostic error "-Wdiv-by-zero"
#pragma GCC diagnostic error "-Wendif-labels"
#pragma GCC diagnostic error "-Wfree-nonheap-object"
#pragma GCC diagnostic error "-Whardened"
#pragma GCC diagnostic error "-Wif-not-aligned"
#pragma GCC diagnostic error "-Wignored-attributes"
#pragma GCC diagnostic error "-Wint-to-pointer-cast"
#pragma GCC diagnostic error "-Winvalid-memory-model"
#pragma GCC diagnostic error "-Wlto-type-mismatch"
#pragma GCC diagnostic error "-Wmissing-profile"
#pragma GCC diagnostic error "-Wodr"
#pragma GCC diagnostic error "-Wopenmp"
#pragma GCC diagnostic error "-Woverflow"
#pragma GCC diagnostic error "-Wpointer-compare"
#pragma GCC diagnostic error "-Wpragmas"
#pragma GCC diagnostic error "-Wprio-ctor-dtor"
#pragma GCC diagnostic error "-Wpsabi"
#pragma GCC diagnostic error "-Wreturn-local-addr"
#pragma GCC diagnostic error "-Wscalar-storage-order"
#pragma GCC diagnostic error "-Wshift-count-negative"
#pragma GCC diagnostic error "-Wshift-count-overflow"
#pragma GCC diagnostic error "-Wsizeof-array-argument"
#pragma GCC diagnostic error "-Wstringop-overread"
#pragma GCC diagnostic error "-Wswitch-bool"
#pragma GCC diagnostic error "-Wswitch-outside-range"
#pragma GCC diagnostic error "-Wswitch-unreachable"
#pragma GCC diagnostic error "-Wsync-nand"
#pragma GCC diagnostic error "-Wtsan"
#pragma GCC diagnostic error "-Wterminate"
#pragma GCC diagnostic error "-Wunicode"
#pragma GCC diagnostic error "-Wvarargs"
#pragma GCC diagnostic error "-Wxor-used-as-pow"

#ifndef __cplusplus
#pragma GCC diagnostic error "-Werror-implicit-function-declaration"
#pragma GCC diagnostic error "-Wcompare-distinct-pointer-types"
#pragma GCC diagnostic error "-Wdeclaration-missing-parameter-type"
#pragma GCC diagnostic error "-Wdesignated-init"
#pragma GCC diagnostic error "-Wdiscarded-array-qualifiers"
#pragma GCC diagnostic error "-Wdiscarded-qualifiers"
#pragma GCC diagnostic error "-Wincompatible-pointer-types"
#pragma GCC diagnostic error "-Wint-conversion"
#pragma GCC diagnostic error "-Woverride-init-side-effects"
#pragma GCC diagnostic error "-Wpointer-to-int-cast"
#pragma GCC diagnostic error "-Wreturn-mismatch"
#endif


// -Wall
#pragma GCC diagnostic error "-Waddress"
#pragma GCC diagnostic error "-Warray-compare"
#pragma GCC diagnostic error "-Warray-parameter=2"
#pragma GCC diagnostic error "-Wbool-compare"
#pragma GCC diagnostic error "-Wbool-operation"
#pragma GCC diagnostic error "-Wcatch-value"
#pragma GCC diagnostic error "-Wchar-subscripts"
#pragma GCC diagnostic error "-Wclass-memaccess"
#pragma GCC diagnostic error "-Wcomments"
#pragma GCC diagnostic error "-Wdangling-else"
#pragma GCC diagnostic error "-Wdangling-pointer=2"
#pragma GCC diagnostic error "-Wdelete-non-virtual-dtor"
#pragma GCC diagnostic error "-Wenum-compare"
#pragma GCC diagnostic error "-Wformat=1"
#pragma GCC diagnostic error "-Wformat-contains-nul"
#pragma GCC diagnostic error "-Wformat-diag"
#pragma GCC diagnostic error "-Wformat-extra-args"
#pragma GCC diagnostic error "-Wformat-overflow=1"
#pragma GCC diagnostic error "-Wformat-truncation=1"
#pragma GCC diagnostic error "-Wformat-zero-length"
#pragma GCC diagnostic error "-Wframe-address"
#pragma GCC diagnostic error "-Winfinite-recursion"
#pragma GCC diagnostic error "-Winit-self"
#pragma GCC diagnostic error "-Wint-in-bool-context"
#pragma GCC diagnostic error "-Wlogical-not-parentheses"
#pragma GCC diagnostic error "-Wmain"
#pragma GCC diagnostic error "-Wmaybe-uninitialized"
#pragma GCC diagnostic error "-Wmemset-elt-size"
#pragma GCC diagnostic error "-Wmemset-transposed-args"
#pragma GCC diagnostic error "-Wmisleading-indentation"
#pragma GCC diagnostic error "-Wmismatched-dealloc"
#pragma GCC diagnostic error "-Wmismatched-new-delete"
#pragma GCC diagnostic error "-Wmissing-attributes"
#pragma GCC diagnostic error "-Wmissing-braces"
#pragma GCC diagnostic error "-Wmultistatement-macros"
#pragma GCC diagnostic error "-Wnarrowing"
#pragma GCC diagnostic error "-Wnonnull"
#pragma GCC diagnostic error "-Wnonnull-compare"
#pragma GCC diagnostic error "-Wopenmp-simd"
#pragma GCC diagnostic error "-Woverloaded-virtual=1"
#pragma GCC diagnostic error "-Wpacked-not-aligned"
#pragma GCC diagnostic error "-Wparentheses"
#pragma GCC diagnostic error "-Wpessimizing-move"
#pragma GCC diagnostic error "-Wrange-loop-construct"
#pragma GCC diagnostic error "-Wreorder"
#pragma GCC diagnostic error "-Wrestrict"
#pragma GCC diagnostic error "-Wreturn-type"
#pragma GCC diagnostic error "-Wself-move"
#pragma GCC diagnostic error "-Wsequence-point"
#pragma GCC diagnostic error "-Wsign-compare"
#pragma GCC diagnostic error "-Wsizeof-array-div"
#pragma GCC diagnostic error "-Wsizeof-pointer-div"
#pragma GCC diagnostic error "-Wsizeof-pointer-memaccess"
#pragma GCC diagnostic error "-Wstrict-aliasing"
#pragma GCC diagnostic error "-Wstrict-overflow=1"
#pragma GCC diagnostic error "-Wswitch"
#pragma GCC diagnostic error "-Wtautological-compare"
#pragma GCC diagnostic error "-Wtrigraphs"
#pragma GCC diagnostic error "-Wuninitialized"
#pragma GCC diagnostic error "-Wunknown-pragmas"
#pragma GCC diagnostic error "-Wunused"
#pragma GCC diagnostic error "-Wunused-but-set-variable"
#pragma GCC diagnostic error "-Wunused-const-variable=1"
#pragma GCC diagnostic error "-Wunused-function"
#pragma GCC diagnostic error "-Wunused-label"
#pragma GCC diagnostic error "-Wunused-local-typedefs"
#pragma GCC diagnostic error "-Wunused-value"
#pragma GCC diagnostic error "-Wunused-variable"
#pragma GCC diagnostic error "-Wuse-after-free=2"
#pragma GCC diagnostic error "-Wvla-parameter"
#pragma GCC diagnostic error "-Wvolatile-register-var"
#pragma GCC diagnostic error "-Wzero-length-bounds"

// Should be in C++ -Wall, but is not
#pragma GCC diagnostic error "-Wdangling-reference"

// -Wextra
#pragma GCC diagnostic error "-Walloc-size"
#pragma GCC diagnostic error "-Wcalloc-transposed-args"
#pragma GCC diagnostic error "-Wcast-function-type"
#pragma GCC diagnostic error "-Wclobbered"
#pragma GCC diagnostic error "-Wdeprecated-copy"
#pragma GCC diagnostic error "-Wempty-body"
#pragma GCC diagnostic error "-Wenum-conversion"
#pragma GCC diagnostic error "-Wexpansion-to-defined"
#pragma GCC diagnostic error "-Wignored-qualifiers"
#pragma GCC diagnostic error "-Wimplicit-fallthrough=3"
#pragma GCC diagnostic error "-Wmaybe-uninitialized"
#pragma GCC diagnostic error "-Wmissing-field-initializers"
#pragma GCC diagnostic error "-Wredundant-move"
#pragma GCC diagnostic error "-Wshift-negative-value"
#pragma GCC diagnostic error "-Wsign-compare"
#pragma GCC diagnostic error "-Wsized-deallocation"
#pragma GCC diagnostic error "-Wstring-compare"
#pragma GCC diagnostic error "-Wtype-limits"
#pragma GCC diagnostic error "-Wuninitialized"
#pragma GCC diagnostic error "-Wunused-but-set-parameter"

// Should be in C++ -Wextra, but is not
#pragma GCC diagnostic error "-Winaccessible-base"

#ifndef __cplusplus
// -Wall C only
#pragma GCC diagnostic error "-Wduplicate-decl-specifier"
#pragma GCC diagnostic error "-Wenum-int-mismatch"
#pragma GCC diagnostic error "-Wimplicit"
#pragma GCC diagnostic error "-Wimplicit-function-declaration"
#pragma GCC diagnostic error "-Wimplicit-int"
#pragma GCC diagnostic error "-Wpointer-sign"
// -Wextra C only
#pragma GCC diagnostic error "-Wabsolute-value"
#pragma GCC diagnostic error "-Wmissing-parameter-type"
#pragma GCC diagnostic error "-Wold-style-declaration"
#pragma GCC diagnostic error "-Woverride-init"
// Ignore, only relevant for C90 or earlier
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"
#endif

static_assert(__cplusplus == 202002L);
#pragma GCC diagnostic error "-Wc++20-compat"
#pragma GCC diagnostic error "-Wc++20-extensions"
#pragma GCC diagnostic ignored "-Wshift-negative-value"
#pragma GCC diagnostic error "-Wdeprecated-enum-enum-conversion"
#pragma GCC diagnostic error "-Wdeprecated-enum-float-conversion"
#pragma GCC diagnostic error "-Wtemplate-id-cdtor"
#pragma GCC diagnostic error "-Welaborated-enum-base"

#pragma GCC diagnostic error "-Walloc-zero"
#pragma GCC diagnostic error "-Wcast-align"
#pragma GCC diagnostic error "-Wcast-qual"
#pragma GCC diagnostic error "-Wconditionally-supported"
#pragma GCC diagnostic error "-Wctor-dtor-privacy"
#pragma GCC diagnostic error "-Wdisabled-optimization"
#pragma GCC diagnostic error "-Wdouble-promotion"
#pragma GCC diagnostic error "-Wduplicated-branches"
#pragma GCC diagnostic error "-Wduplicated-cond"
#pragma GCC diagnostic error "-Weffc++"
#pragma GCC diagnostic error "-Wextra-semi"
#pragma GCC diagnostic error "-Wfloat-equal"
#pragma GCC diagnostic error "-Wformat=2"
#pragma GCC diagnostic error "-Wformat-nonliteral"
#pragma GCC diagnostic error "-Wformat-security"
#pragma GCC diagnostic error "-Wformat-signedness"
#pragma GCC diagnostic error "-Wformat-y2k"
#pragma GCC diagnostic error "-Winvalid-pch"
#pragma GCC diagnostic error "-Wlogical-op"
#pragma GCC diagnostic error "-Wmissing-declarations"
#pragma GCC diagnostic error "-Wmissing-format-attribute"
#pragma GCC diagnostic error "-Wmissing-include-dirs"
#pragma GCC diagnostic error "-Wmissing-noreturn"
#pragma GCC diagnostic error "-Wnamespaces"
#pragma GCC diagnostic error "-Wnoexcept"
#pragma GCC diagnostic error "-Wnull-dereference"
#pragma GCC diagnostic error "-Woverloaded-virtual"
#pragma GCC diagnostic error "-Wpacked"
#pragma GCC diagnostic error "-Wpedantic"
#pragma GCC diagnostic error "-Wpointer-arith"
#pragma GCC diagnostic error "-Wredundant-decls"
#pragma GCC diagnostic error "-Wregister"
#pragma GCC diagnostic error "-Wstack-protector"
#pragma GCC diagnostic error "-Wstrict-flex-arrays"
#pragma GCC diagnostic error "-Wstrict-null-sentinel"
#pragma GCC diagnostic error "-Wstrict-overflow=2"
#pragma GCC diagnostic error "-Wsuggest-attribute=const"
#pragma GCC diagnostic error "-Wsuggest-attribute=format"
#pragma GCC diagnostic error "-Wsuggest-attribute=malloc"
#pragma GCC diagnostic error "-Wsuggest-final-types"
#pragma GCC diagnostic error "-Wsuggest-final-methods"
#pragma GCC diagnostic error "-Wsuggest-override"
#pragma GCC diagnostic error "-Wswitch-default"
#pragma GCC diagnostic error "-Wswitch-enum"
#pragma GCC diagnostic error "-Wtrampolines"
#pragma GCC diagnostic error "-Wundef"
#pragma GCC diagnostic error "-Wunreachable-code"
#pragma GCC diagnostic error "-Wunsafe-loop-optimizations"
#pragma GCC diagnostic error "-Wunused"
#pragma GCC diagnostic error "-Wuseless-cast"
#pragma GCC diagnostic error "-Wvariadic-macros"
#pragma GCC diagnostic error "-Wvla"
#pragma GCC diagnostic error "-Wvla-larger-than=0"
#pragma GCC diagnostic error "-Wwrite-strings"
#pragma GCC diagnostic error "-Wzero-as-null-pointer-constant"
#pragma GCC diagnostic error "-Wunused-result"

#pragma GCC diagnostic error "-Wframe-larger-than=4096"
#pragma GCC diagnostic error "-Wstack-usage=4096"

// signed/unsigned
#pragma GCC diagnostic error "-Wconversion"
#pragma GCC diagnostic error "-Wfloat-conversion"
#pragma GCC diagnostic error "-Wsign-conversion"
#pragma GCC diagnostic error "-Wsign-promo"

// expect pain
#pragma GCC diagnostic error "-Wformat-overflow=2"
#pragma GCC diagnostic error "-Wold-style-cast"
#pragma GCC diagnostic error "-Wshadow"


#pragma GCC diagnostic error "-Walloca"
#pragma GCC diagnostic error "-Walloca-larger-than=0"

// Only relevant if compiled with -fanalyzer
#pragma GCC diagnostic error "-Wanalyzer-symbol-too-complex"
#pragma GCC diagnostic error "-Wanalyzer-too-complex"

#pragma GCC diagnostic error "-Wattribute-alias=2"

#ifndef __cplusplus
#pragma GCC diagnostic error "-Wbad-function-cast"
#pragma GCC diagnostic error "-Wjump-misses-init"
#pragma GCC diagnostic error "-Wmissing-prototypes"
#pragma GCC diagnostic error "-Wmissing-variable-declarations"
#pragma GCC diagnostic error "-Wnested-externs"
#pragma GCC diagnostic error "-Wold-style-definition"
#pragma GCC diagnostic error "-Wstrict-prototypes"
#endif

#pragma GCC diagnostic error "-Wbidi-chars=any"
#pragma GCC diagnostic error "-Wbidi-chars=ucn"
#pragma GCC diagnostic error "-Wcast-align=strict"
#pragma GCC diagnostic error "-Wcatch-value=3"
#pragma GCC diagnostic error "-Wchanges-meaning"
#pragma GCC diagnostic error "-Warray-bounds=3"
#pragma GCC diagnostic error "-Warray-parameter=3"
#pragma GCC diagnostic error "-Waligned-new=all"
#pragma GCC diagnostic error "-Wchkp"
#pragma GCC diagnostic error "-Wclass-conversion"
#pragma GCC diagnostic error "-Wcomma-subscript"
#pragma GCC diagnostic error "-Wcomment"
#pragma GCC diagnostic error "-Wctad-maybe-unsupported"
#pragma GCC diagnostic error "-Wdate-time"

#pragma GCC diagnostic error "-Wdeprecated-copy-dtor"

// Should be enabled by default but is not?
#pragma GCC diagnostic error "-Wdelete-incomplete"
#pragma GCC diagnostic error "-Wmultichar"
#pragma GCC diagnostic error "-Wstringop-truncation"

#pragma GCC diagnostic error "-Wflex-array-member-not-at-end"
#pragma GCC diagnostic error "-Whsa"
#pragma GCC diagnostic error "-Winit-list-lifetime"
#pragma GCC diagnostic error "-Winline"
#pragma GCC diagnostic error "-Winterference-size"
#pragma GCC diagnostic error "-Winvalid-constexpr"
#pragma GCC diagnostic error "-Winvalid-imported-macros"
#pragma GCC diagnostic error "-Winvalid-utf8"
#pragma GCC diagnostic error "-Wmismatched-tags"
#pragma GCC diagnostic error "-Wnormalized=nfkc"
#pragma GCC diagnostic error "-Wopenacc-parallelism"
#pragma GCC diagnostic error "-Wplacement-new=2"
#pragma GCC diagnostic error "-Wshift-overflow=2"
#pragma GCC diagnostic error "-Wstringop-overflow=3"
#pragma GCC diagnostic error "-Wtrivial-auto-var-init"
#pragma GCC diagnostic error "-Wunused-macros"
#pragma GCC diagnostic error "-Wvector-operation-performance"
#pragma GCC diagnostic error "-Wvirtual-move-assign"
#pragma GCC diagnostic error "-Wvolatile"

// ignored
// We build the complete firmware and don't care about abi.
#pragma GCC diagnostic ignored "-Wabi"
#pragma GCC diagnostic ignored "-Wabi-tag"
#pragma GCC diagnostic ignored "-Wnoexcept-type"

// https://stackoverflow.com/a/14017047
#pragma GCC diagnostic ignored "-Waggregate-return"

// From -Wextra, creates a lot of errors right now.
#pragma GCC diagnostic ignored "-Wunused-parameter"

// Creates lots of errors. Probably fine to leave disabled?
#pragma GCC diagnostic ignored "-Warith-conversion"

// long long is fine in C++11
#pragma GCC diagnostic ignored "-Wlong-long"

// We allow multiple inheritance
#pragma GCC diagnostic ignored "-Wmultiple-inheritance"

// We don't care about old GCC versions
#pragma GCC diagnostic ignored "-Wnon-template-friend"

// The -Wdelete-non-virtual-dtor option (enabled by -Wall) should be preferred because it warns about the unsafe cases without false positives.
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"

// Creates lots of errors.
#pragma GCC diagnostic ignored "-Wnrvo"

#ifndef __cplusplus
// We don't care about old compilers
#pragma GCC diagnostic ignored "-Woverlength-strings"
#pragma GCC diagnostic ignored "-Wtraditional"
#pragma GCC diagnostic ignored "-Wtraditional-conversion"
#pragma GCC diagnostic ignored "-Wunsuffixed-float-constants"
#endif

// Creates lots of errors.
#pragma GCC diagnostic ignored "-Wpadded"

// Creates lots of errors.
#pragma GCC diagnostic ignored "-Wredundant-tags"

// We don't care about compatibility with cfront :D
#pragma GCC diagnostic ignored "-Wsynth"

// We don't care about warnings in system headers
#pragma GCC diagnostic ignored "-Wsystem-headers"

// Templates are allowed
#pragma GCC diagnostic ignored "-Wtemplates"

// Virtual inheritance is allowed
#pragma GCC diagnostic ignored "-Wvirtual-inheritance"

// frame-larger-than should already catch all interesting cases
#pragma GCC diagnostic ignored "-Wlarger-than-"
#pragma GCC diagnostic ignored "-Wlarger-than="

#endif

// Both gcc and clang understand GCC poison
#if defined(__GNUC__)
#pragma GCC poison millis
#pragma GCC poison micros
#endif
