# 1 "/tmp/ctags-capture/sketches/BridgeExample/BridgeExample.ino"
/*
  Arduino Yún Bridge example

  This example for the Arduino Yún shows how to use the
  Bridge library to access the digital and analog pins
  on the board through REST calls. It demonstrates how
  you can create your own API when using REST style
  calls through the browser.

  Possible commands created in this shetch:

  "/arduino/digital/13"     -> digitalRead(13)
  "/arduino/digital/13/1"   -> digitalWrite(13, HIGH)
  "/arduino/analog/2/123"   -> analogWrite(2, 123)
  "/arduino/analog/2"       -> analogRead(2)
  "/arduino/mode/13/input"  -> pinMode(13, INPUT)
  "/arduino/mode/13/output" -> pinMode(13, OUTPUT)

  This example code is part of the public domain

  http://www.arduino.cc/en/Tutorial/Bridge

*/

# 26 "/tmp/ctags-capture/sketches/BridgeExample/BridgeExample.ino" 2
# 27 "/tmp/ctags-capture/sketches/BridgeExample/BridgeExample.ino" 2
# 28 "/tmp/ctags-capture/sketches/BridgeExample/BridgeExample.ino" 2

// Listen to the default port 5555, the Yún webserver
// will forward there all the HTTP requests you send
BridgeServer server;

void setup() {
  // Bridge startup
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  Bridge.begin();
  digitalWrite(13, HIGH);

  // Listen for incoming connection only from localhost
  // (no one from the external network could connect)
  server.listenOnLocalhost();
  server.begin();
}

void loop() {
  // Get clients coming from server
  BridgeClient client = server.accept();

  // There is a new client?
  if (client) {
    // Process request
    process(client);

    // Close connection and free resources.
    client.stop();
  }

  delay(50); // Poll every 50ms
}

void process(BridgeClient client) {
  // read the command
  String command = client.readStringUntil('/');

  // is "digital" command?
  if (command == "digital") {
    digitalCommand(client);
  }

  // is "analog" command?
  if (command == "analog") {
    analogCommand(client);
  }

  // is "mode" command?
  if (command == "mode") {
    modeCommand(client);
  }
}

void digitalCommand(BridgeClient client) {
  int pin, value;

  // Read pin number
  pin = client.parseInt();

  // If the next character is a '/' it means we have an URL
  // with a value like: "/digital/13/1"
  if (client.read() == '/') {
    value = client.parseInt();
    digitalWrite(pin, value);
  } else {
    value = digitalRead(pin);
  }

  // Send feedback to client
  client.print((reinterpret_cast<const __FlashStringHelper *>(("Pin D"))));
  client.print(pin);
  client.print((reinterpret_cast<const __FlashStringHelper *>((" set to "))));
  client.println(value);

  // Update datastore key with the current pin value
  String key = "D";
  key += pin;
  Bridge.put(key, String(value));
}

void analogCommand(BridgeClient client) {
  int pin, value;

  // Read pin number
  pin = client.parseInt();

  // If the next character is a '/' it means we have an URL
  // with a value like: "/analog/5/120"
  if (client.read() == '/') {
    // Read value and execute command
    value = client.parseInt();
    analogWrite(pin, value);

    // Send feedback to client
    client.print((reinterpret_cast<const __FlashStringHelper *>(("Pin D"))));
    client.print(pin);
    client.print((reinterpret_cast<const __FlashStringHelper *>((" set to analog "))));
    client.println(value);

    // Update datastore key with the current pin value
    String key = "D";
    key += pin;
    Bridge.put(key, String(value));
  } else {
    // Read analog pin
    value = analogRead(pin);

    // Send feedback to client
    client.print((reinterpret_cast<const __FlashStringHelper *>(("Pin A"))));
    client.print(pin);
    client.print((reinterpret_cast<const __FlashStringHelper *>((" reads analog "))));
    client.println(value);

    // Update datastore key with the current pin value
    String key = "A";
    key += pin;
    Bridge.put(key, String(value));
  }
}

void modeCommand(BridgeClient client) {
  int pin;

  // Read pin number
  pin = client.parseInt();

  // If the next character is not a '/' we have a malformed URL
  if (client.read() != '/') {
    client.println((reinterpret_cast<const __FlashStringHelper *>(("error"))));
    return;
  }

  String mode = client.readStringUntil('\r');

  if (mode == "input") {
    pinMode(pin, INPUT);
    // Send feedback to client
    client.print((reinterpret_cast<const __FlashStringHelper *>(("Pin D"))));
    client.print(pin);
    client.print((reinterpret_cast<const __FlashStringHelper *>((" configured as INPUT!"))));
    return;
  }

  if (mode == "output") {
    pinMode(pin, OUTPUT);
    // Send feedback to client
    client.print((reinterpret_cast<const __FlashStringHelper *>(("Pin D"))));
    client.print(pin);
    client.print((reinterpret_cast<const __FlashStringHelper *>((" configured as OUTPUT!"))));
    return;
  }

  client.print((reinterpret_cast<const __FlashStringHelper *>(("error: invalid mode "))));
  client.print(mode);
}
