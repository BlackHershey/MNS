/*
  MNS stimulation pulse generator

  This sketch receives Serial instructions to send TTL pulses to an iWorx stimulator.

  The circuit:
  * iWorx TTL input connected to a digital I/O pin specified below

  Jon Koller
  kollerj@wustl.edu

*/

int ttlPin = 12;      // the number of the TTL pin
unsigned long pulseWidth; // pulse width in microseconds of on-time
float frequency;    // pulse frequency in Hz
float periodUs;     // period in microseconds
unsigned long numPulses; // number of Pulses left to send
unsigned long pulseCounter; // number of Pulses sent so far
unsigned long nextPulseTime;
char statusMessage[100];
const int numArrhythmicPulses10Hz = 600;
const int numArrhythmicPulses12Hz = 720;
float tPause10Hz[numArrhythmicPulses10Hz];
float tPause12Hz[numArrhythmicPulses12Hz];
unsigned long tPause_index;

// These maintain the current state
int ttlState;                 // ttlState used to set the TTL
unsigned long currentMicros;
unsigned long startMicros; //stores when starting new pulse train

int int_input = 0;
String string_input;
String pulse_type;
bool rhythmic_stim;
int ind1, ind2, ind3;

float sum_less_than_p01(float tPause_array[], int arraySize){
  float sum = 0.0;
  int i;

  for ( i = 0; i < arraySize; i++ ){
    if ( tPause_array[i] < 0.01 ){
      sum += tPause_array[i];
    }
  }
  return sum;
}

void setup() {
  // put your setup code here, to run once:
  pulseWidth = 0UL;
  frequency = 0.0;
  numPulses = 0UL;
  pulseCounter = 0UL;
  ttlState = LOW;
  startMicros = 0UL;
  pinMode(ttlPin,OUTPUT);
  digitalWrite(ttlPin,ttlState);
  Serial.begin(115200);

  // create arrhythmic pulse train similarly to Nottingham matlab code
  randomSeed(1);
  int i = 0;
  int j = 0;
  float rc = 0.0;
  float first_rc = 0.0;
  float minute_delay_scale = 59.2;
  float sumDelays10Hz, sumDelays12Hz, sumA, sumAa;

  // 10 Hz pulse delays
  sumA = 0.0;
  for ( i = 0; i < numArrhythmicPulses10Hz-1; i++ ){
    tPause10Hz[i] = float(random(0,1000001))/float(1000000);
    sumA += tPause10Hz[i];
  }
  // Serial.print("sum(A) = ");
  // Serial.println(sumA, 6);

  for ( i = 0; i < numArrhythmicPulses10Hz-1; i++ ){
    tPause10Hz[i] = minute_delay_scale * (tPause10Hz[i]/sumA);
    sumDelays10Hz += tPause10Hz[i];
  }

  // Serial.print("sum(tPause10Hz) = ");
  // Serial.println(sumDelays10Hz, 6);
  rc = sum_less_than_p01(tPause10Hz, numArrhythmicPulses10Hz-1);
  first_rc = rc;
  // Serial.print("first rc = ");
  // Serial.println(first_rc, 6);

  // Refine tPause so no Delay is less than 0.01 seconds
  j = 0;
  while ( rc > 0.0 ){
    j++;
    // Serial.print("j = ");
    // Serial.println(j);
    sumAa = 0.0;
    for ( i = 0; i < numArrhythmicPulses10Hz-1; i++ ){
      if ( tPause10Hz[i] < 0.01 ){
        tPause10Hz[i] = float(random(0,1000001))/float(1000000);
        sumAa += tPause10Hz[i];
        tPause10Hz[i] += 5.0;
      }
    }
    // Serial.print("sum(Aa) = ");
    // Serial.println(sumAa, 6);
    for ( i = 0; i < numArrhythmicPulses10Hz-1; i++ ){
      if ( tPause10Hz[i] > 5.0 ){
        // Serial.print("i = ");
        // Serial.print(i);
        // Serial.print(": tPause old / new = ");
        // Serial.print(tPause10Hz[i], 6);
        tPause10Hz[i] = tPause10Hz[i] - 5.0;
        tPause10Hz[i] = (tPause10Hz[i]/sumAa) * first_rc;
        // Serial.print(" / ");
        // Serial.println(tPause10Hz[i], 6);
      }
    }
    // get new sumAa
    rc = sum_less_than_p01(tPause10Hz, numArrhythmicPulses10Hz-1);
  }

  // check new sum of tPause
  sumDelays10Hz = 0.0;
  for ( i = 0; i < numArrhythmicPulses10Hz-1; i++ ){
    sumDelays10Hz += tPause10Hz[i];
  }
  Serial.print("sum(tPause10Hz) after removing less than 0.01s = ");
  Serial.println(sumDelays10Hz, 6);
  tPause10Hz[numArrhythmicPulses10Hz-1] = 60.0 - (0.0002 * float(numArrhythmicPulses10Hz)) - sumDelays10Hz;

  // 12 Hz pulse delays
  sumA = 0.0;
  for ( i = 0; i < numArrhythmicPulses12Hz-1; i++ ){
    tPause12Hz[i] = float(random(0,1000001))/float(1000000);
    sumA += tPause12Hz[i];
  }
  // sprintf(statusMessage, "sum(A) = %.2f", sumDelays);
  // Serial.print("sum(A) = ");
  // Serial.println(sumA, 6);

  for ( i = 0; i < numArrhythmicPulses12Hz-1; i++ ){
    tPause12Hz[i] = minute_delay_scale * (tPause12Hz[i]/sumA);
    sumDelays12Hz += tPause12Hz[i];
  }

  // Serial.print("sum(tPause12Hz) = ");
  // Serial.println(sumDelays12Hz, 6);
  rc = sum_less_than_p01(tPause12Hz, numArrhythmicPulses12Hz-1);
  first_rc = rc;
  // Serial.print("first rc = ");
  // Serial.println(first_rc, 6);

  // Refine tPause so no Delay is less than 0.01 seconds
  j = 0;
  while ( rc > 0.0 ){
    j++;
    // Serial.print("j = ");
    // Serial.println(j);
    sumAa = 0.0;
    for ( i = 0; i < numArrhythmicPulses12Hz-1; i++ ){
      if ( tPause12Hz[i] < 0.01 ){
        tPause12Hz[i] = float(random(0,1000001))/float(1000000);
        sumAa += tPause12Hz[i];
        tPause12Hz[i] += 5.0;
      }
    }
    // Serial.print("sum(Aa) = ");
    // Serial.println(sumAa, 6);
    for ( i = 0; i < numArrhythmicPulses12Hz-1; i++ ){
      if ( tPause12Hz[i] > 5.0 ){
        // Serial.print("i = ");
        // Serial.print(i);
        // Serial.print(": tPause old / new = ");
        // Serial.print(tPause12Hz[i], 6);
        tPause12Hz[i] = tPause12Hz[i] - 5.0;
        tPause12Hz[i] = (tPause12Hz[i]/sumAa) * first_rc;
        // Serial.print(" / ");
        // Serial.println(tPause12Hz[i], 6);
      }
    }
    // get new sumAa
    rc = sum_less_than_p01(tPause12Hz, numArrhythmicPulses12Hz-1);
  }

  // check new sum of tPause
  sumDelays12Hz = 0.0;
  for ( i = 0; i < numArrhythmicPulses12Hz-1; i++ ){
    sumDelays12Hz += tPause12Hz[i];
  }
  Serial.print("sum(tPause12Hz) after removing less than 0.01s = ");
  Serial.println(sumDelays12Hz, 6);
  tPause12Hz[numArrhythmicPulses12Hz-1] = 60.0 - (0.0002 * float(numArrhythmicPulses12Hz)) - sumDelays12Hz;
  
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available()>0){
    string_input = Serial.readStringUntil('\n');
    ind1 = string_input.indexOf('x');
    pulse_type = string_input.substring(0,ind1);
    if ( pulse_type.indexOf('R') > -1 ){
      rhythmic_stim = true;
    }
    if ( pulse_type.indexOf('A') > -1 ){
      rhythmic_stim = false;
    }
    ind2 = string_input.indexOf('x', ind1+1);
    numPulses = string_input.substring(ind1+1,ind2+1).toInt();
    ind3 = string_input.indexOf('x', ind2+1);
    pulseWidth = string_input.substring(ind2+1,ind3+1).toInt();
    frequency = string_input.substring(ind3+1).toFloat();
    periodUs = 1000000.0 / frequency;
    pulseCounter = 0UL;
    startMicros = micros();
    nextPulseTime = micros();

    // check that arrhythmic stim is 10 or 12 Hz
    if ( ! rhythmic_stim ){
      if ( frequency == 10.0 ){
        Serial.println("Arrhythmic stimulation, average frequency = 10 Hz");
      } else if ( frequency == 12.0 ){
        Serial.println("Arrhythmic stimulation, average frequency = 12 Hz");
      } else {
        Serial.println("ERROR: arrhythmic stim must be 10 or 12 Hz");
        numPulses = 0UL;
      }
    }
    
    sprintf(statusMessage, "STARTING %lu PULSES",numPulses);
    Serial.println(statusMessage);
    if (numPulses < 1){
      ttlState = LOW;  // Turn it off
      digitalWrite(ttlPin, ttlState);  // Update the TTL pin
    }
  }
  // ttl.Update();
  // check to see if it's time to change the state of the LED
  currentMicros = micros();
  if(pulseCounter < numPulses){
    // sprintf(statusMessage, "pulseCounter = %lu, numPulses = %lu, currentMicros = %lu",pulseCounter,numPulses,currentMicros);
    // Serial.println(statusMessage);
    // sprintf(statusMessage, "pulseTimes[%lu] = %lu, pulseWidth = %lu",pulseCounter,pulseTimes[pulseCounter],pulseWidth);
    // Serial.println(statusMessage);
    if((ttlState == HIGH) && (currentMicros >= nextPulseTime + pulseWidth)){
      ttlState = LOW;  // Turn it off
      digitalWrite(ttlPin, ttlState);  // Update the TTL pin
      pulseCounter++;
      // set next pulse time based on delay from last pulse (either rhythmic based on frequencey or arrhythmic based on randomly-generated tPause array
      if ( rhythmic_stim ){
        nextPulseTime = (unsigned long) round(float(pulseCounter) * periodUs);
        nextPulseTime += startMicros;
      } else {
        if ( frequency == 12.0 ){
          tPause_index = ( pulseCounter - 1 ) % numArrhythmicPulses12Hz;
          nextPulseTime += pulseWidth + (unsigned long) round(tPause12Hz[tPause_index]*1000000.0);
        } else if ( frequency == 10.0 ){
          tPause_index = ( pulseCounter - 1 ) % numArrhythmicPulses10Hz;
          nextPulseTime += pulseWidth + (unsigned long) round(tPause10Hz[tPause_index]*1000000.0);
        }
      }
    }
    else if ((ttlState == LOW) && (currentMicros >= nextPulseTime)){
      ttlState = HIGH;  // turn it on
      digitalWrite(ttlPin, ttlState);   // Update the TTL pin
    }
  } else {
    ttlState = LOW;  // Turn it off
    digitalWrite(ttlPin, ttlState);
  }
}
