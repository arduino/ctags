// Macros. ctags sees the gcc -E output, so these are expanded before
// tagging.

#define DECLARE_GETTER(name, value) \
  int get_##name() {                \
    return value;                   \
  }

DECLARE_GETTER(width, 10)
DECLARE_GETTER(height, 20)

#define WEAK __attribute__((weak))
#define RETURN_TYPE unsigned int

WEAK void weakHook() {
}

RETURN_TYPE macroReturnType() {
  return 1;
}

#if defined(EDGE_UNDEFINED)
int disabledFunction() {
  return 0;
}
#else
int enabledFunction() {
  return 1;
}
#endif

#ifdef __cplusplus
extern "C" {
#endif
int cLinkage(int a) {
  return a;
}
#ifdef __cplusplus
}
#endif

[[nodiscard]] int attributeStandard() {
  return 1;
}

int noexceptFunction() noexcept {
  return 1;
}

void setup() {
  get_width();
  get_height();
  weakHook();
  macroReturnType();
  enabledFunction();
  cLinkage(1);
  (void) attributeStandard();
  noexceptFunction();
}

void loop() {
}
