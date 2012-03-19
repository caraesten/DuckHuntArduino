/*
/
/  Duck Hunt!
/  -By Esten Hurtle, 2012
/  -For Scott Hudson's Applied Gadgets class
/
/  Duck Hunt is copyright Nintendo, 1984
/  This is a fan remake and all that jazz.
/
/  If Nintendo gets upset, replace "Duck Hunt" with
/  "AMAZING LIGHTGUN MINI-VIDEO-GAME EXTRAVAGANZA" :)
/
/  I've never done anything with electronics before,
/  so apologies for my code, which is probably not that
/  great. I'm a n00b. Especially with bit-shifting and
/  manipulating registers. I like high-level things like
/  Python most of the time. Until now, bits scared me.
/
*/

#include <avr/io.h>
#include <math.h>

#define WIDTH 8
#define HEIGHT 8

const int columnPins[] = {2,3,4,5,6,7,8,9};
const int rowPins[] = {A0, A1, A2, A3, A4, A5, 11, 12};
const int photoPin = A6;
const int triggerPin = 10;
int iteration;
int canFire;
int curCol;
int deadDuckCount;
int lastButtonState;
int inputIterations;
int logicIterations;
const int debounceDelay = 10;
long lastDebounceTime;
const int logicDelay = 15;
const int inputDelay = 5;
boolean twoDuckMode = true;


int onMatrix[8][8] = {
  {1,1,1,1,1,1,1,1},
  {1,1,1,1,1,1,1,1},
  {1,1,1,1,1,1,1,1},
  {1,1,1,1,1,1,1,1},
  {1,1,1,1,1,1,1,1},
  {1,1,1,1,1,1,1,1},
  {1,1,1,1,1,1,1,1},
  {1,1,1,1,1,1,1,1}
};

int shotMatrix[8][8] = {
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0}
};

int bigTargetMatrix[8][8] = {
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0}
};


int score[WIDTH][HEIGHT] = {
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0}
};


/*   This doesn't  really look
      like a dog, but it's the
      best I can do given a tiny
      number of pixels :/ */
int dog[8][8] = {
  {0,0,1,1,1,1,0,0},
  {0,1,0,1,0,0,1,0},
  {0,1,0,0,1,1,0,1},
  {1,1,1,0,0,0,0,1},
  {1,1,1,0,0,0,0,1},
  {0,1,0,0,1,1,0,1},
  {0,1,0,1,0,0,1,0},
  {0,0,1,1,1,1,0,0}
};

/* The percent delta (20%) between
    an all-bright matrix, and a 
    target */
float deltaPercentHit = 0.20;

class Duck
{
  private:
    float _speed_x;
    float _speed_y;
    float _x;
    float _y;

  public:
    boolean shot;
    boolean on_screen;
    boolean oppositeDuck;
    
    // Duck constructor. Set all variables to initial positions
    Duck() {
      _speed_x = 0;
      _speed_y = 0;
      _y = 0;
      _x = 0;
      shot = false;
      on_screen = false;
      oppositeDuck = false;
    };
    
    // Duck will go in opposite direction
    void makeOpposite (){
      oppositeDuck = true;
    };
    
    // Change x value.
    void setX(int new_x) {
      if (_x >= 0 && _x < WIDTH){
        _x = new_x;
      }
    };
    
    // Change y value.
    void setY(int new_y){
      if (_y >= 0 && _y < HEIGHT){
        _y = new_y;
      }
    };
    
    /*  Check if it's displayed, can
        also be accessed through on_screen
        public var */
    boolean isDisplayed(){
      return on_screen;
    };
    
    /* Check if it's shot. Can also
       be accessed through shot public
       var. */
    boolean isShot(){
      return shot;
    };
    
    /* Puts the duck on the screen, with
       random speed and position. */
    void showDuck(){
     _speed_x = ((float) random(2,10) / 100.0);
     _speed_y = ((float) random(10,30) / 1000.0);
     _y = random(2,6);
     if (!oppositeDuck){
       _x = 0;
     }
     else{
       _x = WIDTH - 1;       
     }
     shot = false;
     on_screen = true;
    };
    
    /* Resets the duck to original, pre-displayed
       status */
    void resetDuck(){
      _speed_x = 0;
      _speed_y = 0;
      _y = 0;
      if (!oppositeDuck){
        _x = 0;
      }
      else {
        _x = 0;
      }
      shot = true;
      on_screen = false;
    };
    
    /* Marks duck as shot, but does nothing else */
    void shootDuck() {
      shot = true; 
    };
    
    /* Revive duck, without displaying it */
    void reviveDuck(){
      _speed_x = 0;
      _speed_y = 0;
      _y = 0;
      _x = 0;
      shot = false;
      on_screen = false;
    };
    
    /* Execute a move step */
    void moveTarget(){
      if (!oppositeDuck){
        _x += _speed_x;
        _y += _speed_y;
      }
      else {
        _x -= _speed_x;
        _y += _speed_y;
      }
    };
    
    /* return the X coordinate */
    int getX(){
      if (_x >= 0 && _x < WIDTH){
        // Since it's a float, cast to int and floor it.
        return (int) floor(_x);
      }
      else {
        // if it's off screen, return -1
        return -1;
      }
    };
    
    /* return the Y coordinate */
    int getY(){
      if (_y >= 0 && _y < HEIGHT){
        return (int) floor(_y);
      }
      else {
        return -1;
      };
    };
};

Duck duck; //goose

Duck duck2;


void resetAll(){
  // Bring ducks back to life for later
  duck.reviveDuck();
  duck2.reviveDuck();
  
  // Reset count of killed or offscreen ducks
  deadDuckCount = 0;
  
  // Reset iteration
  iteration = 0;
  
  // Gun can shoot
  canFire = 1;
  
  // Column to draw is the first one
  curCol = 0;
  
  // Reset logic and input iterations
  logicIterations = 0;
  inputIterations = 0;
  
  // Flip two duck mode
  twoDuckMode = !twoDuckMode;
  
  // Clear matrix of shots
  clearShots();
  Serial.print("State reset");
}

void setup(){
  // debounce, save last button state
  lastButtonState = LOW;
  
  // set direction of registers
  DDRC |= B111111;
  DDRB |= B011000;
  
  // set column pins using normal arduino code,
  // since registers aren't super helpful here
  for (int i = 0; i < WIDTH; i++){
    pinMode(columnPins[i], OUTPUT);
    digitalWrite(columnPins[i], HIGH);
  }
  
  // Handle input
  pinMode(photoPin, INPUT);
  pinMode(triggerPin, INPUT);
  
  // Create ducks, make the second one the opposite
  duck = Duck();
  duck2 = Duck();
  duck2.makeOpposite();
  Serial.begin(9600);
  
  // Run all code that resets the game every time
  resetAll();



}

void clearColumn(int col){
  // Turn column pins high
  
  // Port for analog pins

  PORTC |= B111111;

  // Port for two digital pins
  

  PORTB |= B011000;

  
  digitalWrite(columnPins[col], LOW);
}


// This function goes through and makes every pixel touching
// the target pixel shoot-able. Otherwise, this game is WAY
// TOO HARD.
void makeBigTarget(){
  // Loop through the matrix
  for (int x=0; x <WIDTH; x++){
    for (int y=0; y < HEIGHT; y++){
      /* Go through each touching pixel and turn it on */
      if (shotMatrix[x][y] == 1){
        if (x != 0 && y != 0){
          bigTargetMatrix[x-1][y-1] = 1;
        }
        if (x != 0){
          bigTargetMatrix[x-1][y] = 1;
        }
        if (y != 0){
          bigTargetMatrix[x][y-1] = 1;
        }
        if (x < WIDTH - 1 && y < HEIGHT - 1){
          bigTargetMatrix[x+1][y+1] = 1;
        }
        if (x < WIDTH - 1){
          bigTargetMatrix[x+1][y] = 1; 
        }
        if (y < HEIGHT - 1){
          bigTargetMatrix[x][y+1] = 1; 
        }
        if (x != 0 && y < HEIGHT - 1){
          bigTargetMatrix[x-1][y+1] = 1;
        }
        if (x < WIDTH - 1 && y != 0){
          bigTargetMatrix[x + 1][y - 1] = 1; 
        }
        bigTargetMatrix[x][y] = 1;
      }
      else {
        bigTargetMatrix[x][y] = 0;
      }
    }
  }
}

/* This is based off of Nintendo's original
   NES Zapper patent. It works by flashing
   the screen all light, recording the resistance
   on the photoresistor, then flashing all
   light except for the duck target and
   recording that resistance. It sees if
   the change is greater than the delta
   previously set, then it's a hit.  */
int detectHit(){
  /* Start by averaging resistance for the "on" state. We 
     don't know where the photoresistor is (what column). 
     If you average every column, including the dark ones 
     with the light, the resistance will still be higher when 
     the light one is included, reflecting that state  */
     
  /* This could also be done with a list of the resistance values
     on each individual column. This might be a better system,
     but the one I'm using here is simpler, since it doesn't
     need to keep track of where the "scan line" is until it hits
     the actual duck hit calculation. That said, I'll probably
     do a list-based system later, since it just makes more sense. */
  

  int columnVal[WIDTH] = {0};
  for (int i = 0; i < WIDTH; i++){
    digitalWrite(columnPins[i], HIGH);
    for (int y = 0; y < HEIGHT; y++){
        digitalWrite(rowPins[y], LOW);
    }
    columnVal[i] = analogRead(photoPin);
    clearColumn(i);
  }
  
  // Create the large target, making it easier to hit
  makeBigTarget();
  
  /* This is the 'did I hit it?' calculation */
  for (int x = 0; x < WIDTH; x++){
    digitalWrite(columnPins[x], HIGH);
    for (int y = 0; y < HEIGHT; y++){
      // If this is a targetable pixel, turn it on.
      if (bigTargetMatrix[x][y] == 0){
        digitalWrite(rowPins[y], LOW);
      }
      else {
        digitalWrite(rowPins[y], HIGH);
      }
      // Check to see if the position of the "scan line" is in the right place, if the resistance difference indicates a hit
      if (analogRead(photoPin) < columnVal[x] - (deltaPercentHit * columnVal[i]) && bigTargetMatrix[x][y] == 1){
        // Check to see if the current position of the duck is within two pixels of the current "scan line" position.
        // This isn't wholly necessary, but should cover any weirdness with correspondence between matrix and duck
        if((abs(x - duck.getX()) < 2 && abs(y - duck.getY()) < 2)){
          // Change score
          for (int j = 0; j < HEIGHT; j++){
            if (score[WIDTH - 1][j] != 1){
              score[WIDTH - 1][j] = 1;
              // Indicate the first duck was hit
              return 1;
            }
          }
        }
        // Do the same for the second duck, if need be
        if (twoDuckMode == true && abs(x - duck2.getX()) < 2 && abs(y - duck2.getY()) < 2){
          for (int j = 0; j < HEIGHT; j++){
            if (score[WIDTH - 1][j] != 1){
              score[WIDTH - 1][j] = 1;
              return 2;
            }
          }
        } 
      }
    }
    // Turn off the column
    digitalWrite(columnPins[x], LOW);
  }
  return -1;
}

// clear off the shot matrix
void clearShots(){
  for (int x = 0; x < WIDTH; x++){
   for (int y = 0; y < HEIGHT; y++){
     shotMatrix[x][y] = 0;
   }
  }
}

void drawColumn(int matrix[WIDTH][HEIGHT], int col, boolean invert){
  
  // Turn on the column
  digitalWrite(columnPins[col], HIGH);
  
  // Check if inverted (probably won't happen, but keeping code in just in case)
  int onDigit = 1;
  if (invert == true){
    onDigit = 0;
  }
  
  // Port for analog pins
  for (int i = 0; i < HEIGHT - 2; i++){
    if (matrix[col][i] == onDigit){
      // Turn on the pins
      PORTC ^= (1 << i);
    }
  }
  
  // Port for two digital pins

  for (int j = 3; j < 5; j++){
    if (matrix[col][j+3] == onDigit){
      PORTB ^= (1 << j);
    }
  }
  
  // Keep it on for a bit
  delayMicroseconds(800);
  
  // Turn off
  clearColumn(col);

}

boolean checkInput(){
  // This does debouncing and checking input
  int fired = digitalRead(triggerPin);
  boolean debounced = false;
  if ((millis() - lastDebounceTime) > debounceDelay){
    debounced = true;
  }
  // If the debounce has resulted in a state change...
  if (canFire == 1 && debounced == 1 && fired== HIGH){
    if (fired != lastButtonState){
      lastDebounceTime = millis();
    }
    Serial.print("FIRED!\n");
    
    // Can't fire for a bit
    canFire = 0;
    
    // Check if there's a hit
    int hit = detectHit();
    if(hit != -1){
      // First duck...
      if (hit == 1){
        // Make sure it's displayed before clearing it
        // (this was mostly a saftey thing from when
        // I was using interrupts)
        if (duck.isDisplayed()){
          // Clear the duck array
          clearShots();
          // reset the duck
          duck.resetDuck();
          // increment the counter
          deadDuckCount += 1;
        }
      }
      if (hit == 2){
        if (duck2.isDisplayed()){
          clearShots();
          duck2.resetDuck();
          deadDuckCount += 1;
        }
      }
    }
    else {
      Serial.print("MISS!\n");
    }
    // Input has happened.
    lastButtonState = fired;
    return true;
  }
  else {
    lastButtonState = fired;
    return false;
  }
}

void doGameOver(){

    Serial.print("Handling game over...");
    
    // Display the dog for 1000 iterations through the loop
    for (int r = 0; r < 1000; r++){
      for (int i = 0; i < WIDTH; i++){
        drawColumn(dog, i, false);
      }
    }
    
    // Reset score
    for (int j = 0; j < HEIGHT; j++){
      score[WIDTH - 1][j] = 0;
    }
    Serial.print("Game restarting...");
    
    // Clear the ducks array
    clearShots();
    // Reset everything
    resetAll();
}

void loop(){
  // Check to see if input should be checked
  if (inputIterations > inputDelay){
    inputIterations = 0;
    checkInput();
  }
  // Check to see if the game should end
  if (deadDuckCount >= 8){
    Serial.print("Game should be ending...");
    doGameOver();
  }
  
  // Check to see if logic should happen
  if (logicIterations > logicDelay){
    logicIterations = 0;
    doGameLogic();
  }
  // check to see if score column needs to be drawn, if not, draw normal column from shot matrix
  if (curCol < 7){
    drawColumn(shotMatrix, curCol, false);
    curCol += 1;
  }
  else{
    // Draw the last column as score
    drawColumn(score, 7, false);
    curCol = 0;
  }
  
  // Iterate input and logic
  inputIterations += 1;
  logicIterations += 1;
  
}


// This is the game mechanics loop.
void doGameLogic()
{
  // Clear shots array
  clearShots();
  
  // Iterate logic
  iteration += 1;
  
  // Reset the ability to shoot
  if (iteration == 50){
    canFire = 1;
  }
  
  // Spawn a new duck
  if (iteration >= 200){
    Serial.print("\n\nTwo Duck Mode:");
    Serial.print(twoDuckMode);
    iteration = 0;
    Serial.print("\nDeadDuckCount: ");
    Serial.print(deadDuckCount);
    // If the duck is still on screen, kill it and iterate the dead duck count
    if (duck.isDisplayed()){
      deadDuckCount += 1;
    }
    duck.showDuck();
    // If we're in the game mode with two ducks, show the other duck
    if (twoDuckMode == true){
      if (duck2.isDisplayed()){
        deadDuckCount += 1;
      }
      duck2.showDuck();
      Serial.print("Making duck2");
    }
  }
  if (duck.isDisplayed()){
      int x = duck.getX();
      int y = duck.getY();
      // Make sure the duck isn't off-screen or shot
      if (x != -1 && y != -1 && !duck.isShot()){
        // Move the duck
        duck.moveTarget();
        // Is it off screen now?
        if (x != -1 && y != -1){
          // Get the new x and y and show the duck
          x = duck.getX();
          y = duck.getY();
          // Set the duck's position in the matrix
          shotMatrix[x][y] = 1;
        }
        else {
          // Kill the duck, reset it, increment counter
          duck.resetDuck();
          deadDuckCount += 1;
        }
      }
      else {
        duck.resetDuck();
        deadDuckCount += 1;
      }
  }
  // If there's two ducks, do the same for the second                                                                                               
  if (twoDuckMode == true && duck2.isDisplayed()){
      int x = duck2.getX();
      int y = duck2.getY();
      if (x != -1 && y != -1 && !duck2.isShot()){
        duck2.moveTarget();
        if (x != -1 && y != -1){
          x = duck2.getX();
          y = duck2.getY();
          shotMatrix[x][y] = 1;
        }
        else {
          duck2.resetDuck();
          deadDuckCount += 1;
        }
      }
      else {
        duck2.resetDuck();
        deadDuckCount += 1;
      }
  }
}
