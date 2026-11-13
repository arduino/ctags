# 1 "/tmp/ctags-capture/sketches/EdgeReturnTypes/EdgeReturnTypes.ino"
// Return types and qualifiers prototype generation has to reproduce.

struct Point {
  int x;
  int y;
};

enum class Mode { Off, On };

unsigned long long bigValue() {
  return 1ULL << 40;
}

const char *name() {
  return "edge";
}

const char * const constName() {
  return "edge";
}

int &counter() {
  static int value = 0;
  return value;
}

Point origin() {
  return Point{0, 0};
}

Point *pointArray(size_t n) {
  return new Point[n];
}

Mode currentMode() {
  return Mode::On;
}

static int helper(int a) {
  return a * 2;
}

inline int twice(int a) {
  return a + a;
}

static inline unsigned char lowByte8(unsigned int v) {
  return v & 0xff;
}

constexpr int square(int a) {
  return a * a;
}

auto trailing(int a) -> long {
  return a;
}

unsigned
splitReturnType(unsigned a) {
  return a;
}

void setup() {
  counter() = helper(twice(square(lowByte8(splitReturnType(3)))));
  trailing(bigValue() > 0);
  name();
  constName();
  origin();
  delete[] pointArray(2);
  currentMode();
}

void loop() {
}
