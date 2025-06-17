/*=============================================================================  
 * full_board.ino
 *   
 * This sketch showcases 3 activities going on simultaneously on an Uno: 
 *    - continuous servo, controled by a poteniometer
 *    - playing a song on a speaker, controlled by a button
 *    - driving an RGB LED pattern.
 *    
 */

/*====================================================
 *Servo defines 
 ====================================================*/
#include "Servo.h"

#define SERVOPIN 13 

Servo myServo;

typedef enum
{
  SERVO_FWD,
  SERVO_BACK,
  SERVO_STOP
} servo_dir_type;

servo_dir_type servo_dir=SERVO_STOP;

#define SERVO_DIAL_PIN A0

/*====================================================
 * music defines 
 ====================================================*/

#define SONG_BUTTON_PIN 4
#define SPEAKER_PIN     9

bool     song_playing = false;
uint32_t song_start_ms;
uint32_t song_next_note_ms;
int      song_current_note_index;

/* This section of #defines sets the frequency for each note in Hz*/
#define NOTE_REST 0
#define NOTE_D3  147
#define NOTE_C4  262
#define NOTE_Db4 277
#define NOTE_D4  294
#define NOTE_Eb4 311
#define NOTE_E4  330
#define NOTE_F4  340
#define NOTE_Gb4 370
#define NOTE_G4  392
#define NOTE_Ab4 413
#define NOTE_A4  440
#define NOTE_Bb4 466
#define NOTE_B4  493
#define NOTE_C5  523
#define NOTE_Db5 554
#define NOTE_D5  587
#define NOTE_Eb5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_Gb5 739
#define NOTE_G5  783
#define NOTE_Ab5 830
#define NOTE_A5  880
#define NOTE_Bb5 932
#define NOTE_B5  987
#define NOTE_C6  1046
#define NOTE_Db6 1109

/* To define a song, we need a collection of notes.  
 * Each note has a frequency (from the #defines above) and a duration
 */
typedef struct
{
  int freq; 
  int duration_ms;
} note_type;

/* This structure will then hold the array of notes to make a song. 
 * By specifying the number of notes in the song we're able have that song on "repeat" 
 */
typedef struct
{
  note_type *notes;
  int        num_notes;
} song_type;

/* Here are the array definitions for the songs themselves */
note_type scale[]=
{
  {NOTE_C5, 1000},
  {NOTE_D5, 1000},
  {NOTE_E5, 1000},
  {NOTE_F5, 1000},
  {NOTE_G5, 1000},
  {NOTE_A5, 1000},
  {NOTE_B5, 1000},
  {NOTE_C6, 1000}
};

note_type groove[]=
{
  {NOTE_Ab4, 300},
  {NOTE_REST, 100},
  {NOTE_Gb5, 200},
  {NOTE_Eb5, 200},
  {NOTE_Ab5, 200},
  {NOTE_Gb5, 200},
  {NOTE_Eb5, 200},
  {NOTE_Db5, 200},
  {NOTE_REST,200},
  {NOTE_B5,  200},
  {NOTE_Ab5, 200},
  {NOTE_Db6, 200},
  {NOTE_B5,  200},
  {NOTE_Ab5, 200},
  {NOTE_Gb5, 200},
  {NOTE_Ab5, 200} 
};

note_type temple[] =
{
  {NOTE_A4, 100},
  {NOTE_REST,50},
  {NOTE_A4, 100},
  {NOTE_REST,50},
  {NOTE_E5, 100},
  {NOTE_REST,50},
  {NOTE_E5, 100},
  {NOTE_REST,50},
  {NOTE_A5, 100},
  {NOTE_REST,50},
  {NOTE_A5, 100},
  {NOTE_REST,50},
  {NOTE_E5, 100},
  {NOTE_REST,50},
  {NOTE_E5, 100},
  {NOTE_REST,50},
  {NOTE_G4, 100},
  {NOTE_REST,50},
  {NOTE_G4, 100},
  {NOTE_REST,50},
  {NOTE_D5, 100},
  {NOTE_REST,50},
  {NOTE_D5, 100},
  {NOTE_REST,50},
  {NOTE_G5, 100},
  {NOTE_REST,50},
  {NOTE_G5, 100},
  {NOTE_REST,50},
  {NOTE_D5, 100},
  {NOTE_REST,50},
  {NOTE_D5, 100},
  {NOTE_REST,50},
  {NOTE_D4, 100},
  {NOTE_REST,50},
  {NOTE_D4, 100},
  {NOTE_REST,50},
  {NOTE_A4, 100},
  {NOTE_REST,50},
  {NOTE_A4, 100},
  {NOTE_REST,50},
  {NOTE_D5, 100},
  {NOTE_REST,50},
  {NOTE_D5, 100},
  {NOTE_REST,50},
  {NOTE_E5, 100},
  {NOTE_REST,50},
  {NOTE_E5, 100}, 
  {NOTE_REST,50},
  {NOTE_F5, 100},
  {NOTE_REST,50},
  {NOTE_F5, 100},
  {NOTE_REST,50},
  {NOTE_E5, 100},
  {NOTE_REST,50},
  {NOTE_E5, 100},
  {NOTE_REST,50},
  {NOTE_D5, 100},
  {NOTE_REST,50},
  {NOTE_D5, 100},
  {NOTE_REST,50},
  {NOTE_E5, 100},
  {NOTE_REST,50},
  {NOTE_E5, 100},
  {NOTE_REST,50}
};

note_type jaws[] =
{
  {NOTE_B4, 900},
  {NOTE_REST,100},
  {NOTE_C5, 900},
  {NOTE_REST,100} 
};

note_type just_beep[] =
{
  {NOTE_E5,   700},
  {NOTE_REST, 300}
};

/* We now use the above-defined song_type to put the song and number of notes in that song */
song_type groove_song={groove, sizeof(groove)/sizeof(note_type)};
song_type temple_song={temple, sizeof(temple)/sizeof(note_type)};
song_type scale_song={scale, 8};
song_type just_beep_song={just_beep, sizeof(just_beep)/sizeof(note_type)};
song_type jaws_song={jaws, sizeof(jaws)/sizeof(note_type)};
song_type alarm_song;

/* this array is a collection of all the songs we want to play */
song_type all_songs[]=
{
  groove_song,
  temple_song,
  scale_song,
  jaws_song,
  just_beep_song
};

int num_songs = sizeof(all_songs)/sizeof(song_type);

/* These varibles alow us to loop over the songs in the "all_songs" array. */
int current_song_index = 0;
song_type song = groove_song;

/* We're debouncing the "song press" button to remove spurious toggles.
 *  That button needs to be held down for SONG_PRESS_BUTTON_TIME milliseconds and then released
 *  before we detect it as an actual button press.
 */
uint32_t song_button_press_ms = 0;
int      last_song_button_state = 1;
#define SONG_BUTTON_PRESS_TIME 500

/*=================================
 * LED Defines
 ===================================*/
#include <FastLED.h>

#define LED_PIN 6
#define LED_TYPE WS2812
#define COLOR_ORDER GRB
#define BRIGHTNESS 100
#define NUM_LEDS 28

/* CRGB is a type used by the FastLED library.  It lets us set color for each LED individualy.
 *  The leds array keeps track of what color is on each pixel of our LED collection.
 */
CRGB leds[NUM_LEDS];

/* We're going to be doing a moving pattern, so we want to keep track of time.  
 *  This allows us to control the speed of our LED pattern without using the 
 *  arduino's "delay" functions....as using delay would interfere with playing the song.
 */
uint32_t last_pixel_update_ms = 0;
#define PIXEL_UPDATE_RATE_MS 100

/* in the current implementation, we have two rings of LEDs connected to the LED pin:
 *  First, a "big ring" with 16 pixels, then a "small ring" with 12 pixels. 
 *  Inside of the leds array, indexes 0 through 15 will reference the "big ring" pixels,
 *  and 16 through 27 will reference the "small ring" pixels.
 */
#define BIG_RING_START 0
#define BIG_RING_SIZE 16
#define SMALL_RING_START 16
#define SMALL_RING_SIZE 12

/* our pattern is going to be two circling red pixels on a blue background.  We use these
 *  variables to determine the current position of the red LED
 */
int big_ring_red_pos = BIG_RING_START;
int small_ring_red_pos = SMALL_RING_START;


/*=================================
 * MUSIC FUNCTIONS
 ===================================*/

/*=================================
 * start_song
 * 
 * This function starts playing the current song (referenced by the "song" global variable
 * by playing the first note and storing when the next note needs to be played.
 ===================================*/
void start_song( void )
{
  note_type first_note;

  Serial.println("Start Song!!!");
  song_playing = true;

  song_start_ms = millis();

  // play the first note
  first_note = song.notes[0];
  if (first_note.freq == NOTE_REST)
  {
    noTone(SPEAKER_PIN);
  }
  else
  {
    tone(SPEAKER_PIN,first_note.freq);
  }
  
  // mark when the next note should start.
  song_next_note_ms = song_start_ms + first_note.duration_ms;

  // check for special case:  one note song.
  if (song.num_notes == 1) song_current_note_index = 0;
  else song_current_note_index = 1;
}

/*=================================
 * stop_song
 * 
 * This function stops playing the current song (referenced by the "song" global variable
 * by turning off the speaker. 
 * 
 * In addition, we "queue up" the next song by modifing the "song" global variable.
 ===================================*/
void stop_song( void )
{
  noTone(SPEAKER_PIN);
  song_playing = false;

  /* get the next song in the list primed and ready */
  current_song_index++;
  if (current_song_index >= num_songs)
  {
    current_song_index = 0;
  }
  song = all_songs[current_song_index];
}

/*=================================
 * play_song
 * 
 * This function does the heavy lifting of playing the current song.
 * 
 * If it's time for a note to be played, it does so, and then marks the time
 * that the next note needs to be played.
 ===================================*/
void play_song( void )
{
  uint32_t current_time;
  note_type  note;
  
  current_time = millis();
 
  // is it time to play the next note?
  if (current_time > song_next_note_ms)
  {
    note = song.notes[song_current_note_index];
    if (note.freq == NOTE_REST)
    {
      noTone(SPEAKER_PIN);
    }
    else
    {
      tone(SPEAKER_PIN,note.freq);
    }
    
    // set up for the next note
    song_next_note_ms = current_time + note.duration_ms;
    song_current_note_index++;
    if (song_current_note_index == song.num_notes)
    {
      song_current_note_index = 0;
    } 
  }
}

/*=================================
 * check_for_song_toggle
 * 
 * This function debounces the SONG_BUTTON_PIN by keeping track of the button state, and,
 * if the button is pressed, at what time was that button pressed.  
 * 
 * When the button is released, we check to see how long it was pressed for.  If it was 
 * long enough, we'll either start the current song (if no song was playing) or 
 * stop the current song (if a song is currently playing).
 ===================================*/
void check_for_song_toggle(void)
{
  int current_song_button_state;
  int current_ms;

  current_ms = millis();
  
  current_song_button_state = digitalRead(SONG_BUTTON_PIN);
  if (current_song_button_state == 0)
  { 
    /* if the button was just pressed, mark the time when it was pressed */
    if (last_song_button_state == 1)
    {
      Serial.print("Song button pressed.  Time: ");
      Serial.println(current_ms);
      song_button_press_ms = current_ms;
    }
  }
  else
  {
    if (last_song_button_state == 0)
    {
      /* if the button was released, see if it was down long enough to warrant 
       *  a toggle
       */
      Serial.println("button release");
      
      if (current_ms > song_button_press_ms + SONG_BUTTON_PRESS_TIME)
      {
        Serial.println("toggle");
        
        /* toggle the song */
        if (song_playing)
        {
          stop_song();
        }
        else
        {
          start_song();
        }
      }
    }
  }
  
  last_song_button_state = current_song_button_state;
}

/*======================================================
* SERVO FUNCITONS
=======================================================*/

/*=================================
 * check_for_servo_dir_change
 * 
 * We're using a poteniometer attached to the SERVO_DIAL_PIN to control our 
 * continuous servo.  Depending on how that poteniometer is turned, we'll either rotate
 * it forward or backwards, or stop it if the dial is in the middle.
 ===================================*/
void check_for_servo_dir_change( void )
{
  int read_val;

  /* This read will return a value of 0-1023, depending on the value of the poteniometer. */
  read_val = analogRead(SERVO_DIAL_PIN);

  /* Map that dial value to one of three states */
  if (read_val > 600)
  {
    /* Values bigger than 600 correspond to "forward" direction.  
     * If we weren't already moving forward, start the servo moving forward.
     */
    if (servo_dir != SERVO_FWD)
    {
      Serial.println("Starting Servo Forward");
      servo_dir = SERVO_FWD;
      myServo.attach(SERVOPIN);
      myServo.write(0);   // With a continuous servo, values of 0 correspond to full speed 
                          // in one direction.
    }
  }
  else if (read_val < 300)
  {
    /* Values bigger than 600 correspond to "forward" direction.  
     * If we weren't already moving forward, start the servo moving forward.
     */
    if (servo_dir != SERVO_BACK)
    {
      Serial.println("Starting Servo Backwards");
      servo_dir = SERVO_BACK;
      myServo.attach(SERVOPIN);
      myServo.write(180);  // Servo values of 180 correspond to full speed in the other direction.
    }
  }
  else
  {
    /* If the potentiometer is in the middle, stop the servo */
    if (servo_dir != SERVO_STOP)
    {
      Serial.println("Stopping Servo");
      servo_dir = SERVO_STOP;
      myServo.detach();
    }
  }
  
}

/*=====================================
 * LED FUNCTIONS
 ======================================*/

/*=================================
 * rotate_rings
 * 
 * We've got two RGB rings in our project...a large one with 16 pixels, and a smaller one
 * with 12 pixels.
 * 
 * This function causes both rings to be mostly blue, with one red LED that chases around
 * each ring.  Since the rings map to different places in the "leds" array, we need to do 
 * some math to figure out where in the ring we need to update.
 ===================================*/
void rotate_rings(void)
{
  uint32_t current_ms;
  int i;
  
  current_ms = millis();

  /* is it time for an update? */
  if (current_ms > last_pixel_update_ms + PIXEL_UPDATE_RATE_MS)
  {
    /* Update the big ring */
    for (i = BIG_RING_START; i < BIG_RING_SIZE; i++)
    {
      /* Mark the current index (big_ring_red_pos) as red, and the others in the big ring blue */
      if (i == big_ring_red_pos)
      {
        leds[i] = CRGB::Red;
      }
      else
      {
        leds[i] = CRGB::Blue;
      }
    }
    /* and then move our "red" position one around, dealing with the fact that when we get to the 
     *  end of the ring, we need to go back to zero (the start of the ring).
     */
    big_ring_red_pos++;
    if (big_ring_red_pos == BIG_RING_SIZE)
    {
      big_ring_red_pos = 0;
    }

    /* Update the small ring */

    /* inside of the "leds" array, the small rings positions are 16 through 27.  */
    for (i = SMALL_RING_START; i < (SMALL_RING_START + SMALL_RING_SIZE); i++)
    {
      /* Same thing going on here as with the big ring.  Set one pixel of the small ring to 
       *  red, and the rest blue
       */
      if (i == small_ring_red_pos)
      {
        leds[i] = CRGB::Red;
      }
      else
      {
        leds[i] = CRGB::Blue;

      }
    }
    /* ...and advance where that red pixel will be next time...wrapping it around now
     *  to the index marking the start of the small ring (16)
     */
    small_ring_red_pos++;
    if (small_ring_red_pos == (SMALL_RING_SIZE + SMALL_RING_START) )
    {
      small_ring_red_pos = SMALL_RING_START;
    }

    /* Now that we've got the leds array set, FastLED.show() actually displays those pixels */
    FastLED.show();
    last_pixel_update_ms = current_ms;
  }
      
}

/*==================================
 * Setup....runs once at start
 ===================================*/
void setup (void)
{
  int i;
  
  Serial.begin(9600);
  
  pinMode(SPEAKER_PIN, OUTPUT);
  pinMode(SONG_BUTTON_PIN, INPUT_PULLUP);

  myServo.attach(SERVOPIN);
  pinMode(SERVO_DIAL_PIN, INPUT);

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );

  Serial.println("Initialized...starting power-up-test");
  
  // Power up test...cycle all LEDs blue.
  for (i=0;i<NUM_LEDS;i++)
  {
    leds[i] = CRGB::Blue;
    FastLED.show();
    FastLED.delay(100);
  }

  Serial.println("power up test complete");


}

/*=========================================
 * Loop...continuously runs
 ==========================================*/
void loop (void)
{

  check_for_song_toggle();
  if (song_playing)
  {
    play_song();
  }

  check_for_servo_dir_change();

  rotate_rings();
  

}
