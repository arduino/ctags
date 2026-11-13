# 1 "/tmp/ctags-capture/sketches/EdgeParameters/EdgeParameters.ino"
// Parameter lists: types declared in the sketch, pointers, arrays,
// references, callbacks and parameters split over several lines.

struct Config {
  int rate;
};

enum Color { RED, GREEN };

typedef void (*handler_t)(int);

void useStruct(Config cfg) {
  (void) cfg;
}

void useStructRef(const Config &cfg) {
  (void) cfg;
}

void useEnum(Color c) {
  (void) c;
}

void useArray(int values[], size_t n) {
  (void) values;
  (void) n;
}

void useMatrix(int m[3][4]) {
  (void) m;
}

void useArrayRef(int (&values)[8]) {
  (void) values;
}

void useCallback(void (*cb)(int), int arg) {
  cb(arg);
}

void useTypedefCallback(handler_t cb) {
  cb(0);
}

int (*returnsCallback(int which))(int) {
  (void) which;
  return nullptr;
}

void useRvalue(Config &&cfg) {
  (void) cfg;
}

void useVariadic(const char *fmt, ...) {
  (void) fmt;
}

void useUnnamed(int, float) {
}

void useVoid(void) {
}

void splitParameters(int first,
                     const char *second,
                     Config third) {
  (void) first;
  (void) second;
  (void) third;
}

void splitAfterParen(
    int a,
    int b) {
  (void) a;
  (void) b;
}

void braceOnNextLine(int a)
{
  (void) a;
}

void setup() {
  Config cfg = {1};
  int values[8] = {0};
  int m[3][4] = {{0}};
  useStruct(cfg);
  useStructRef(cfg);
  useEnum(GREEN);
  useArray(values, 8);
  useMatrix(m);
  useArrayRef(values);
  useCallback(nullptr, 0);
  useTypedefCallback(nullptr);
  returnsCallback(0);
  useRvalue(Config{2});
  useVariadic("%d", 1);
  useUnnamed(1, 2.0f);
  useVoid();
  splitParameters(1, "two", cfg);
  splitAfterParen(1, 2);
  braceOnNextLine(1);
}

void loop() {
}
