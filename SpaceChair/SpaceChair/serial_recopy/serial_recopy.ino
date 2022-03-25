void setup()
{
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  // Leotracker has connection to the gps power pin
  pinMode(8, OUTPUT);
  digitalWrite(8, LOW);

  // start serial port at 9600 bps:
  Serial.begin(115200);
  Serial1.begin(115200);
}

void loop()
{
  if (Serial.available() > 0) {
    Serial1.write(Serial.read());
  }
  if (Serial1.available() > 0) {
    Serial.write(Serial1.read());
  }
}
