// Classes, namespaces and operators: members must not get prototypes,
// free functions after them must.

namespace outer {
namespace inner {
int nested(int a) {
  return a;
}
}
}

namespace a::b {
int compact(int v) {
  return v;
}
}

class Sensor {
public:
  Sensor(int pin) : pin(pin) {}
  ~Sensor() {}
  int read() const {
    return pin;
  }
  virtual void reset() {}
  static Sensor create(int pin) {
    return Sensor(pin);
  }
  bool operator==(const Sensor &other) const {
    return pin == other.pin;
  }
  explicit operator bool() const {
    return pin >= 0;
  }

private:
  int pin;
  void privateHelper();
};

void Sensor::privateHelper() {
}

struct Derived : public Sensor {
  Derived() : Sensor(1) {}
  void reset() override {}
};

Sensor operator+(const Sensor &a, const Sensor &b) {
  return Sensor(a.read() + b.read());
}

int afterClasses(Sensor &s) {
  auto lambda = [](int v) { return v + 1; };
  auto capture = [&s]() -> int { return s.read(); };
  return lambda(capture());
}

void setup() {
  Sensor s = Sensor::create(2);
  Derived d;
  outer::inner::nested(1);
  a::b::compact(2);
  afterClasses(s);
  (void) (s == d);
  (void) (s + d);
}

void loop() {
}
