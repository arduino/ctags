# 1 "/tmp/ctags-capture/sketches/EdgeMultipleFiles/EdgeMultipleFiles.ino"
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
# 1 "/tmp/ctags-capture/sketches/EdgeMultipleFiles/display.ino"
const char *lastMessage = nullptr;

void displayShow(const char *message) {
  lastMessage = message;
  displayRefresh();
}

void displayRefresh() {
}
# 1 "/tmp/ctags-capture/sketches/EdgeMultipleFiles/motor.ino"
int motorSpeed = 0;

void motorStart(int speed) {
  motorSpeed = speed;
}

void motorStop() {
  motorSpeed = 0;
}
