# 1 "/tmp/ctags-capture/sketches/EdgeTemplates/EdgeTemplates.ino"
// Templates: specializations, variadics, default arguments and
// non-type parameters.

template <typename T>
T maxOf(T a, T b) {
  return a > b ? a : b;
}

template <>
int maxOf<int>(int a, int b) {
  return a > b ? a : b;
}

template <typename T, size_t N>
size_t countOf(T (&)[N]) {
  return N;
}

template <typename T = int, int Scale = 10>
T scaled(T v) {
  return v * Scale;
}

template <typename... Args>
int countArgs(Args... args) {
  return sizeof...(args);
}

template <typename T>
struct Box {
  T value;
  T get() const;
};

template <typename T>
T Box<T>::get() const {
  return value;
}

template <class A,
          class B>
A convert(B b) {
  return static_cast<A>(b);
}

int shifted(int v) {
  return (v >> 1) + (v << 2);
}

bool compare(int a, int b) {
  return a < b && b > a;
}

void setup() {
  int values[4] = {0};
  maxOf(1.0f, 2.0f);
  maxOf(1, 2);
  countOf(values);
  scaled(3);
  countArgs(1, 2, 3);
  Box<int> box = {5};
  box.get();
  convert<int>(2.5);
  shifted(8);
  compare(1, 2);
}

void loop() {
}
