// Dennis Fleurbaaij 2016 - MIT license
// S0 PCM counter for Arduino

const unsigned int S0PCM_ID = 1;

const byte DIGITAL_SENSOR_PIN = 12;
const byte PULSE_LED_PIN = 13; // Onboard led

const unsigned long REPORT_INTERVAL_MS = 10000UL;
const unsigned long DEBOUNCE_TIME_MS = 1UL;

char buf[64];
unsigned long pulse_count = 0;


// Called once by by runtime
void setup()
{
  Serial.begin(9600);

  sprintf(buf, "/%u:S0PCM Pulse Counter", S0PCM_ID);
  Serial.println(buf);

  pinMode(DIGITAL_SENSOR_PIN, INPUT);
  pinMode(PULSE_LED_PIN, OUTPUT);
}


// Debounced measurement of the sensor pin.
void measure(const unsigned long now)
{
  static unsigned long ignore_read_until_timestamp = 0;
  static unsigned long pulse_start = 0;
  static int last_val = LOW;
  
  if(ignore_read_until_timestamp >= now)
    return;
  
  const int val = digitalRead(DIGITAL_SENSOR_PIN);
  if(last_val != val)
  {
    if(val == LOW) // On
    {
      digitalWrite(PULSE_LED_PIN, HIGH);

      pulse_start = now;
    }
    else
    {
      digitalWrite(PULSE_LED_PIN, LOW);

      const unsigned long duration = now - pulse_start;
      if(duration >= 20 && duration <= 110)
        ++pulse_count;
    }

    last_val = val;
    ignore_read_until_timestamp = now + DEBOUNCE_TIME_MS;
  }
}


// Output measurements in S0PCM format every REPORT_INTERVAL_MS.
void output(const unsigned long now)
{
  static unsigned long next_output_timestamp = now + REPORT_INTERVAL_MS;
  static unsigned long total_pulse_count = 0;
  
  if(now >= next_output_timestamp)
  {
    total_pulse_count += pulse_count;

    sprintf(buf, "ID:%u:I:%lu:M1:%lu:%lu", 
        S0PCM_ID, REPORT_INTERVAL_MS/1000UL, pulse_count, total_pulse_count);
    Serial.println(buf);

    pulse_count = 0;
    next_output_timestamp += REPORT_INTERVAL_MS;
  }
}

// Called continuously by runtime. 
void loop() 
{
  const unsigned long now = millis();
  
  measure(now);
  output(now);
}
