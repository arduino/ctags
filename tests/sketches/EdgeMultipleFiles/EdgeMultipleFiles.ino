// Functions spread over several .ino files are called before they are
// defined. arduino-cli merges the files with #line directives, so the
// tags must point at the right file and line.

void setup() {
  motorStart(100);
  displayShow("ready");
}

void loop() {
  motorStop();
}
