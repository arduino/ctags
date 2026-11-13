# 1 "/tmp/ctags-capture/sketches/SketchWithTypename/SketchWithTypename.ino"
template< typename T >
  struct Foo{
    typedef T Bar;
};

void setup() {
  func();
}

void loop() {}

typename Foo<char>::Bar func(){

}
