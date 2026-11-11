# 1 "/tmp/ctags-capture/sketches/EdgeMacros/EdgeMacros.ino"
// Macros. ctags sees the gcc -E output, so these are expanded before
// tagging.






int get_width() { return 10; }
int get_height() { return 20; }




__attribute__((weak)) void weakHook() {
}

unsigned int macroReturnType() {
  return 1;
}






int enabledFunction() {
  return 1;
}



extern "C" {

int cLinkage(int a) {
  return a;
}

}


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
