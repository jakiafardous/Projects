#include <Servo.h>
#include <math.h>

// ============================================================
// PIN DEFINITIONS
// ============================================================

const int greenLED = 2;
const int yellowLED = 3;
const int redLED = 4;

const int buzzer = 6;
const int servoPin = 9;

const int trigPin = 11;
const int echoPin = 12;


// ============================================================
// SERVO
// ============================================================

Servo radarServo;

// ============================================================
// RADAR SETTINGS
// ============================================================

const int MAX_TARGETS = 4;

const int DANGER_DISTANCE = 20;
const int CAUTION_DISTANCE = 30;

// Distance used to group one physical object
// and prevent it from becoming multiple targets.
const float TARGET_RADIUS = 3.0;

// Angle range used when checking an existing target.
const int TARGET_ANGLE_WINDOW = 2;


// ============================================================
// NEW TARGET CONFIRMATION
// ============================================================

// A new target must be detected 3 times before
// it becomes a permanent target.
const int REQUIRED_CONFIRMATIONS = 3;

// Maximum position difference allowed between
// confirmation readings.
const float CANDIDATE_RADIUS = 4.0;


// ============================================================
// ULTRASONIC FILTER
// ============================================================

// Number of ultrasonic readings for each measurement.
const int FILTER_READINGS = 5;

// Ignore extremely large readings.
const int MAX_VALID_DISTANCE = 200;


// ============================================================
// RED LED BLINKING
// ============================================================

const unsigned long RED_BLINK_INTERVAL = 250;

unsigned long lastRedToggle = 0;

bool redLEDState = false;


// ============================================================
// TARGET STRUCTURE
// ============================================================

struct Target {

  bool active;

  int angle;
  int distance;

  float x;
  float y;

  bool checkedThisCycle;
};


// Maximum number of stored targets
Target targets[MAX_TARGETS];


// ============================================================
// CANDIDATE TARGET
// ============================================================

struct Candidate {

  bool active;

  int angle;
  int distance;

  float x;
  float y;

  int confirmations;
};


Candidate candidate;


// ============================================================
// SCANNING
// ============================================================

bool scanningForward = true;

int currentAngle = 0;


// ============================================================
// POSITION FUNCTIONS
// ============================================================

float getX(int angle, int distance) {

  float angleRad = angle * PI / 180.0;

  return distance * cos(angleRad);
}


float getY(int angle, int distance) {

  float angleRad = angle * PI / 180.0;

  return distance * sin(angleRad);
}


float positionDistance(
  float x1,
  float y1,
  float x2,
  float y2
) {

  float dx = x1 - x2;
  float dy = y1 - y2;

  return sqrt(dx * dx + dy * dy);
}


// ============================================================
// FIND TARGET BY PHYSICAL POSITION
// ============================================================

int findTargetByPosition(int angle, int distance) {

  float newX = getX(angle, distance);
  float newY = getY(angle, distance);

  int closestTarget = -1;

  float closestDistance = TARGET_RADIUS + 1;


  for (int i = 0; i < MAX_TARGETS; i++) {

    if (!targets[i].active)
      continue;


    float difference = positionDistance(
      newX,
      newY,
      targets[i].x,
      targets[i].y
    );


    if (
      difference <= TARGET_RADIUS &&
      difference < closestDistance
    ) {

      closestDistance = difference;

      closestTarget = i;
    }
  }


  return closestTarget;
}


// ============================================================
// FIND TARGET NEAR CURRENT ANGLE
// ============================================================

int findTargetNearAngle(int angle) {

  for (int i = 0; i < MAX_TARGETS; i++) {

    if (!targets[i].active)
      continue;


    if (
      abs(targets[i].angle - angle)
      <= TARGET_ANGLE_WINDOW
    ) {

      return i;
    }
  }


  return -1;
}


// ============================================================
// RESET CANDIDATE
// ============================================================

void resetCandidate() {

  candidate.active = false;

  candidate.angle = 0;
  candidate.distance = 0;

  candidate.x = 0;
  candidate.y = 0;

  candidate.confirmations = 0;
}


// ============================================================
// CREATE NEW TARGET
// ============================================================

void createTarget(int angle, int distance) {

  float x = getX(angle, distance);
  float y = getY(angle, distance);


  for (int i = 0; i < MAX_TARGETS; i++) {

    if (!targets[i].active) {

      targets[i].active = true;

      targets[i].angle = angle;
      targets[i].distance = distance;

      targets[i].x = x;
      targets[i].y = y;

      targets[i].checkedThisCycle = true;


      Serial.print("TARGET,");
      Serial.print(i + 1);
      Serial.print(",");

      Serial.print(angle);
      Serial.print(",");

      Serial.print(distance);

      Serial.println(",NEW");


      return;
    }
  }
}


// ============================================================
// UPDATE EXISTING TARGET
// ============================================================

void updateTarget(
  int index,
  int angle,
  int distance
) {

  targets[index].angle = angle;

  targets[index].distance = distance;

  targets[index].x = getX(angle, distance);

  targets[index].y = getY(angle, distance);

  targets[index].checkedThisCycle = true;


  Serial.print("TARGET,");
  Serial.print(index + 1);
  Serial.print(",");

  Serial.print(angle);
  Serial.print(",");

  Serial.print(distance);

  Serial.println(",ACTIVE");
}


// ============================================================
// REMOVE / LOSE TARGET
// ============================================================

void loseTarget(int index) {

  Serial.print("TARGET,");
  Serial.print(index + 1);
  Serial.print(",");

  Serial.print(targets[index].angle);
  Serial.print(",");

  Serial.print(targets[index].distance);

  Serial.println(",LOST");


  targets[index].active = false;

  targets[index].checkedThisCycle = false;
}


// ============================================================
// SINGLE ULTRASONIC READING
// ============================================================

int getSingleDistance() {

  digitalWrite(trigPin, LOW);

  delayMicroseconds(3);


  digitalWrite(trigPin, HIGH);

  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);


  unsigned long duration =
    pulseIn(
      echoPin,
      HIGH,
      30000UL
    );


  // No echo
  if (duration == 0)
    return 400;


  int distance =
    duration * 0.0343 / 2.0;


  // Invalid distance
  if (
    distance < 2 ||
    distance > MAX_VALID_DISTANCE
  ) {

    return 400;
  }


  return distance;
}


// ============================================================
// SORT VALUES
// ============================================================

void sortValues(
  int values[],
  int n
) {

  for (int i = 1; i < n; i++) {

    int key = values[i];

    int j = i - 1;


    while (
      j >= 0 &&
      values[j] > key
    ) {

      values[j + 1] = values[j];

      j--;
    }


    values[j + 1] = key;
  }
}


// ============================================================
// FIVE-READING MEDIAN FILTER
// ============================================================

int getDistance() {

  int values[FILTER_READINGS];


  // Take 5 readings
  for (
    int i = 0;
    i < FILTER_READINGS;
    i++
  ) {

    values[i] = getSingleDistance();


    if (
      i <
      FILTER_READINGS - 1
    ) {

      delay(8);
    }
  }


  // Sort readings
  sortValues(
    values,
    FILTER_READINGS
  );


  // Middle value = median
  int median =
    values[FILTER_READINGS / 2];


  // Invalid median
  if (
    median == 400 ||
    median > MAX_VALID_DISTANCE
  ) {

    return 400;
  }


  return median;
}


// ============================================================
// CHECK IF ANY TARGET IS ACTIVE
// ============================================================

bool anyTargetActive() {

  for (
    int i = 0;
    i < MAX_TARGETS;
    i++
  ) {

    if (targets[i].active)
      return true;
  }


  return false;
}


// ============================================================
// BUZZER + RED LED BLINKING
// ============================================================

void updateBuzzer() {

  // If at least one danger target exists
  if (anyTargetActive()) {

    // Keep buzzer ON
    tone(
      buzzer,
      2000
    );


    // Blink red LED
    unsigned long currentTime =
      millis();


    if (
      currentTime -
      lastRedToggle
      >=
      RED_BLINK_INTERVAL
    ) {

      lastRedToggle =
        currentTime;


      redLEDState =
        !redLEDState;


      digitalWrite(
        redLED,
        redLEDState
          ? HIGH
          : LOW
      );
    }

  }

  else {

    // No danger target
    noTone(buzzer);


    // Turn red LED OFF
    redLEDState = false;

    digitalWrite(
      redLED,
      LOW
    );
  }
}


// ============================================================
// NORMAL DISTANCE LEDS
// ============================================================

void updateLEDs(int distance) {

  digitalWrite(
    greenLED,
    LOW
  );

  digitalWrite(
    yellowLED,
    LOW
  );


  // If a stored danger target exists,
  // red LED is controlled by updateBuzzer().
  if (anyTargetActive()) {

    return;
  }


  // No stored danger target
  digitalWrite(
    redLED,
    LOW
  );


  // DANGER
  if (
    distance > 0 &&
    distance <= DANGER_DISTANCE
  ) {

    digitalWrite(
      redLED,
      HIGH
    );
  }


  // CAUTION
  else if (
    distance > DANGER_DISTANCE &&
    distance <= CAUTION_DISTANCE
  ) {

    digitalWrite(
      yellowLED,
      HIGH
    );
  }


  // SAFE
  else if (
    distance > CAUTION_DISTANCE &&
    distance < 400
  ) {

    digitalWrite(
      greenLED,
      HIGH
    );
  }
}


// ============================================================
// NEW TARGET CONFIRMATION
// ============================================================

void checkDangerTarget(
  int angle,
  int distance
) {

  // Not in danger zone
  if (
    distance <= 0 ||
    distance > DANGER_DISTANCE
  ) {

    resetCandidate();

    return;
  }


  // Check if target already exists
  int existing =
    findTargetByPosition(
      angle,
      distance
    );


  if (existing != -1) {

    updateTarget(
      existing,
      angle,
      distance
    );


    resetCandidate();

    return;
  }


  // Convert to X/Y position
  float x =
    getX(angle, distance);

  float y =
    getY(angle, distance);


  // First confirmation
  if (!candidate.active) {

    candidate.active = true;

    candidate.angle = angle;

    candidate.distance =
      distance;

    candidate.x = x;

    candidate.y = y;

    candidate.confirmations = 1;

    return;
  }


  // Compare with previous candidate position
  float difference =
    positionDistance(
      x,
      y,
      candidate.x,
      candidate.y
    );


  // Same physical location
  if (
    difference <=
    CANDIDATE_RADIUS
  ) {

    candidate.confirmations++;


    candidate.angle =
      angle;

    candidate.distance =
      distance;

    candidate.x = x;

    candidate.y = y;


    // Three confirmations required
    if (
      candidate.confirmations >=
      REQUIRED_CONFIRMATIONS
    ) {

      createTarget(
        angle,
        distance
      );


      resetCandidate();
    }
  }


  // Different object/location
  else {

    candidate.angle =
      angle;

    candidate.distance =
      distance;

    candidate.x = x;

    candidate.y = y;

    candidate.confirmations = 1;
  }
}


// ============================================================
// CHECK STORED TARGET
// ============================================================

void checkStoredTarget(
  int angle,
  int distance
) {

  int targetIndex =
    findTargetNearAngle(
      angle
    );


  if (
    targetIndex == -1 ||
    targets[targetIndex].checkedThisCycle
  ) {

    return;
  }


  // Target still present
  if (
    distance > 0 &&
    distance <= DANGER_DISTANCE
  ) {

    float x =
      getX(angle, distance);

    float y =
      getY(angle, distance);


    float difference =
      positionDistance(
        x,
        y,
        targets[targetIndex].x,
        targets[targetIndex].y
      );


    if (
      difference <=
      TARGET_RADIUS
    ) {

      updateTarget(
        targetIndex,
        angle,
        distance
      );

      return;
    }
  }


  // Target confirmed lost
  loseTarget(
    targetIndex
  );
}


// ============================================================
// RESET TARGET CHECK FLAGS
// ============================================================

void resetTargetCycleFlags() {

  for (
    int i = 0;
    i < MAX_TARGETS;
    i++
  ) {

    if (targets[i].active) {

      targets[i].checkedThisCycle =
        false;
    }
  }
}


// ============================================================
// SETUP
// ============================================================

void setup() {

  pinMode(
    greenLED,
    OUTPUT
  );

  pinMode(
    yellowLED,
    OUTPUT
  );

  pinMode(
    redLED,
    OUTPUT
  );

  pinMode(
    buzzer,
    OUTPUT
  );

  pinMode(
    trigPin,
    OUTPUT
  );

  pinMode(
    echoPin,
    INPUT
  );


  digitalWrite(
    trigPin,
    LOW
  );


  radarServo.attach(
    servoPin
  );


  Serial.begin(
    9600
  );


  // Initialize targets
  for (
    int i = 0;
    i < MAX_TARGETS;
    i++
  ) {

    targets[i].active = false;

    targets[i].angle = 0;

    targets[i].distance = 0;

    targets[i].x = 0;

    targets[i].y = 0;

    targets[i].checkedThisCycle =
      false;
  }


  // Initialize candidate
  resetCandidate();


  // Start servo
  radarServo.write(0);
}


// ============================================================
// MAIN LOOP
// ============================================================

void loop() {

  // Move servo
  radarServo.write(
    currentAngle
  );


  // Servo settling time
  delay(80);


  // Get filtered distance
  int distance =
    getDistance();


  // Check for new danger target
  checkDangerTarget(
    currentAngle,
    distance
  );


  // Check stored targets
  checkStoredTarget(
    currentAngle,
    distance
  );


  // Normal LED indication
  updateLEDs(
    distance
  );


  // Buzzer + persistent red blinking
  updateBuzzer();


  // Send scan data to Python
  Serial.print("SCAN,");
  Serial.print(currentAngle);
  Serial.print(",");
  Serial.println(distance);


  // ========================================================
  // FORWARD SCAN
  // ========================================================

  if (scanningForward) {

    currentAngle += 2;


    if (currentAngle >= 180) {

      currentAngle = 180;

      scanningForward = false;


      resetTargetCycleFlags();
    }
  }


  // ========================================================
  // BACKWARD SCAN
  // ========================================================

  else {

    currentAngle -= 2;


    if (currentAngle <= 0) {

      currentAngle = 0;

      scanningForward = true;


      resetTargetCycleFlags();
    }
  }
}
