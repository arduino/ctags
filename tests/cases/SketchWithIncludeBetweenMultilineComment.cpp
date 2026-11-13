# 1 "/tmp/ctags-capture/sketches/SketchWithIncludeBetweenMultilineComment/SketchWithIncludeBetweenMultilineComment.ino"
# 2 "/tmp/ctags-capture/sketches/SketchWithIncludeBetweenMultilineComment/SketchWithIncludeBetweenMultilineComment.ino" 2
/*
#include <WiFi.h>
*/
CapacitiveSensor cs_13_8 = CapacitiveSensor(13,8);
void setup()
{
 Serial.begin(9600);
}
void loop()
{
 long total1 = cs_13_8.capacitiveSensor(30);
 Serial.println(total1);
 delay(100);
}
