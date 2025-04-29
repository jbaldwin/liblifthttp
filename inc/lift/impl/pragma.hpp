#if defined(__GNUC__) && !defined(__llvm__)
    #define DO_PRAGMA(X)                 _Pragma(#X)
    #define DISABLE_WARNING_PUSH         DO_PRAGMA(GCC diagnostic push)
    #define DISABLE_WARNING_POP          DO_PRAGMA(GCC diagnostic pop)
    #define DISABLE_WARNING(warningName) DO_PRAGMA(GCC diagnostic ignored #warningName)

    #define DISABLE_WARNING_MAYBE_UNINITIALIZED DISABLE_WARNING(-Wmaybe-uninitialized)
#elif defined(__clang__)
    #define DO_PRAGMA(X) _Pragma(#X)
    #define DISABLE_WARNING_PUSH
    #define DISABLE_WARNING_POP
    #define DISABLE_WARNING(warningNumber)

    #define DISABLE_WARNING_MAYBE_UNINITIALIZED
#else
    // unknown compiler, ignoring suppression directives
    #define DISABLE_WARNING_PUSH
    #define DISABLE_WARNING_MAYBE_UNINITIALIZED
    #define DISABLE_WARNING_POP
#endif
