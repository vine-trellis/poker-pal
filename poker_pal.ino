#define checkPin 9
#define callPin 10
#define raisePin 11
#define foldPin 12
#define oddsPin 13

#define analogPin 0

#define interruptPin 2

#define instructionDelay 500 //usually 2000
#define roundDelay 500 //usually 2000
#define roundCountdown 10000 //10 seconds

#define ante 5
#define defaultBet 10

volatile bool Check = false;
volatile bool Bet = false;
volatile bool Raise = false;
volatile bool Fold = false;
volatile bool Odds = false;

float potSize = 0; // does this really need to be volatile?
float wallet = 100.00; //start with 100 dollars?
float betAmount = 0; // does this really need to be volatile?
float currentBet = 10;

unsigned int roundBegin = 0; //timer for the whole round, up to "results"
unsigned int roundDuration = 0;
unsigned int roundEnd = 0;

unsigned int decideTime = 0;

unsigned int actionEnd = 0; //timer for each individual action
unsigned int actionDuration = 0;
unsigned int actionBegin =0;

bool cards[4][13]; //This is the "used cards" array

void setup() {
	pinMode (checkPin, OUTPUT);
	pinMode (callPin, OUTPUT);
	pinMode (raisePin, OUTPUT);
	pinMode (foldPin, OUTPUT);
	pinMode (oddsPin, OUTPUT);


	pinMode(interruptPin, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(interruptPin), userinput, RISING);
	Serial.begin(9600);

	displayInstructions();
	
	//must implement timer scaling
	//OCR0A = 0xAF //this is 175 in decimal
	//TIMSK0 |= _BV(OCIE0A); // need to find out what this does
	

}

void loop() {

	unsigned int flopTime = 0;
	unsigned int turnTime = 0;
	unsigned int riverTime = 0;
	
	resetStates(); //set all states to false just in case
	
	initializeCards();

	roundBegin = millis();
	//PLAYER IS DEALT CARDS
        Serial.println("Okay, here are your cards: ");
        //show cards function
	yourCards();
	anteUp();
        
	while (millis() - roundBegin < roundDelay)
	{}
        
	//PLAYERS SEES FLOP
	Serial.println("Here's the Flop ");
        //show flop function
	//ROUND TIMER STARTS NOW!
	roundBegin = millis();
	
	flop();
	
        Serial.println("Press 'Odds' to see your chance of winning");
        //userinput
	userAction();
	
	flopTime = actionDuration;

  if(!Fold){
	//PLAYER SEES TURN
	Serial.println("The turn is: ");
	//turn function
	turn();
	userAction();
	turnTime = actionDuration;
  }
  

  if(!Fold){
	//PLAYER SEES RIVER
	Serial.println("The river is: ");
	//river function
	river();
	userAction();
	riverTime = actionDuration;
  }
        
	roundEnd = millis();
	roundDuration = roundEnd - roundBegin;
	
        //show results
	showResults(flopTime, turnTime, riverTime);

	
	Serial.print ("\n \n \n ");
  }
  
void anteUp() {

	wallet -= ante;
}
 
void resetStates () {
	betAmount = 0;
	potSize = (ante) * 4;
	Fold = false;
	Check = false;
	Raise = false;
	Bet = false;
	Odds = false;
  actionDuration = 0;
} 
  
void userAction (){
	
	actionBegin = millis();
	
	currentBet = defaultBet;
	
	bool raised;
	
        do
        {
            if(Check)
            {
                  Serial.println ("\nYou pressed Check\n");
                  Check = false;
                  break;
            }
            if(Odds)
            {
              oddsDisplay();
              Odds = false;
            }
            if(Fold)
            {
                Serial.println ("\nYou've Folded! Better luck next time.\n");
                break;
            }
            if(Bet)
            {
		betAmount = currentBet;
              potSize += betAmount;
              Serial.println ("\nYou matched the curent bet!\n");
              Bet = false;
	      wallet -= betAmount;
              break;
            }
            if(Raise)
		{	
			raised = true;
			currentBet += defaultBet;
			Serial.print ("\n You have raised to $");
			Serial.println(currentBet);
			Raise = false;
		}
	
		actionEnd = millis();
 //  Serial.print("end :");
 //  Serial.println(actionEnd);
      	}  while(actionEnd - actionBegin < roundCountdown);  //loop exits if the timer is too timer-y    
        
	actionDuration = actionEnd - actionBegin;
	
	decideTime += actionDuration;

}

void displayInstructions () {
	Serial.println("Welcome to Poker Pal!");
	delay(instructionDelay);
	Serial.println("Test your odds by simulating a real poker game.");
	Serial.println("You will be dealt 2 cards to begin with, you will then see the initial flop.");
	Serial.println("Pressing 'Check' means you are ready to progress the game forward.");
	delay(instructionDelay);
	Serial.println("If you would like to match the pot bet, press 'Bet'.");
	delay(instructionDelay);
	Serial.println("To raise the bet, press 'Raise' until you are satisfied with the raise, then press 'Check'.");
	delay(instructionDelay);
	Serial.println("Pressing 'Fold' will end the game.");
	delay(instructionDelay);
	Serial.println("To find you what the odds are press 'Odds'.");
	delay(instructionDelay);
	Serial.println("You have 10 seconds to make a decisions before Poker Pal ends your turn.");
	delay(instructionDelay * 2);
	Serial.print("\n \n \n \n \n");
}

void showResults (unsigned int flop, unsigned int turn, unsigned int river) {
	Serial.println ("The round is over. \n \n");
	
	Serial.print ("You took ");
	Serial.print ((double)roundDuration/1000.0); // divide by 1000 because of millis
	Serial.println(" seconds to play the round.");
	
	Serial.print ("You took ");
	Serial.print ((double)flop/1000.0);
	Serial.println (" seconds to decide at the flop.");
	
	if(turn !=0)
	{
		Serial.print ("You took ");
		Serial.print ((double)turn/1000.0);
		Serial.println (" seconds to decide at the turn.");
	}
	
	if(river !=0)
	{
		Serial.print ("You took ");
		Serial.print ((double)river/1000.0);
		Serial.println (" seconds to decide at the river.");
	}

 if(wallet > 0){
	
	Serial.println();
	Serial.print ("You have $");
	Serial.print (wallet);
	Serial.println(" remaining.");
 }
 else {
  Serial.print("\nYou ran out of money. I'll give you some more. Please don't become addicted to gambling.\n");
  wallet=1000;
 }
}
    
  
void initializeCards()
{
	for (int i = 0; i <4 ; i ++) // initialize the array
		for (int j = 0; j < 13; j++)
			cards[i][j] = false;
}

void userinput() 
{
int x = analogRead(analogPin); 


	if( x >= 810 && x <= 840)
	{
		Check = true;
	}
	else if( x >= 700 && x <= 740)
	{
		Bet = true;
	}
	else if( x >= 600 && x <= 640)
	{
		Raise = true;
	}
	else if( x >= 520 && x <= 570)
	{
		Fold = true;
	}
	else if( x >= 900)
	{
	Odds = true;
	}
}


void oddsDisplay() {

	Serial.print("\n \n \nPot odds are : ");
	Serial.print(currentBet/potSize);
	Serial.println("%");
	Serial.print(highCard());
	Serial.println("% of holding a high card");
	Serial.print(pair());
	Serial.println("% of making a pair");
	Serial.print(twoPair());
	Serial.println("% of making a two pair");
	Serial.print(threeofaKind());
	Serial.println("% of making a set [three of a kind]");
	Serial.print(straight());
	Serial.println("% of making a straight");
	Serial.print(flush());
	Serial.println("% of making a flush");
	Serial.print(fullHouse());
	Serial.println("% of making a full house");
	Serial.print(straightFlush());
	Serial.println("% of making a straight flush \n \n \n");
}

void potDisplay() {
	Serial.print("Pot size is : $");
	Serial.println(potSize);
	Serial.print("Current bet is : $");
	Serial.println(currentBet);
}


void printCards(int face, int suit) {
	  char suits[4] = {'h','d','c','s'};
	  char faces[13] = {'A', '2', '3', '4', '5', '6', '7', '8', '9', '10', 'J', 'Q' , 'K'};
	  Serial.print(faces[face]);
	  Serial.print(suits[suit]);
	  Serial.print(" ");
}

void getCard(int& suit, int& face){
	suit = random(0,3);
	face = random(0,12);
  
	while(cards[suit][face] == false) {
		if (cards[suit][face] == true) {
			suit = random(0,4);
			face = random(0,14);

		} else 
			cards[suit][face] = true;
	}

}

void yourCards() { //2 cards
  
	//---------- FIRST CARD ------------
	int suit1;
	int face1;
  
	getCard(suit1, face1);
  
  
	//---------- SECOND CARD ------------
	int suit2;
	int face2;
  
	getCard(suit2,face2);

	printCards(face1, suit1); //(int face, int suit)
	Serial.print ("| ");
	printCards(face2, suit2);
	Serial.println();
}


void flop() { //3 cards
  
	int suit[3];
	int face[3];
	
	for (int i = 0; i < 3; i++)
	{
		getCard(suit[i],face[i]);
	}
	
	for (int i = 0; i < 3; i++){
		printCards(face[i],suit[i]);
		if (i < 2)
		Serial.print("| ");
	}

	Serial.println();
    
}
  
  

void turn() { //1 card

	//---------- FIRST CARD ------------
	int suit1;
	int face1;

	getCard(suit1, face1);

	printCards(face1, suit1); //(int face, int suit) 
	Serial.println();
}


void river() { //1 card

	//---------- FIRST CARD ------------
	int suit1;
	int face1;

	getCard(suit1, face1); 

	printCards(face1, suit1); //(int face, int suit) 
	Serial.println();
}


//------- PROBABILITY GENERATORS --------
int highCard() {
	int min = 42; //Inclusive 
	int max = 100; //Exclusive
	
 	int probability = random(min,max);
 	return probability;
}

int pair() {
	int min = 38; //Inclusive 
	int max = 48; //Exclusive
	
 	int probability = random(min,max);
 	return probability;
}

int twoPair() {
	int min = 24; //Inclusive 
	int max = 40; //Exclusive
	
 	int probability = random(min,max);
 	return probability;
}

int threeofaKind() {
	int min = 30; //Inclusive 
	int max = 37; //Exclusive
	
 	int probability = random(min,max);
 	return probability;
}

int straight() {
	int min = 3; //Inclusive 
	int max = 21; //Exclusive
	
 	int probability = random(min,max);
 	return probability;
}

int flush() {
	int min = 2; //Inclusive 
	int max = 18; //Exclusive
	
 	int probability = random(min,max);
 	return probability;
}

int fullHouse() {
	int min = 1; //Inclusive 
	int max = 14; //Exclusive
	
 	int probability = random(min,max);
 	return probability;
}

int straightFlush() {
	int min = 0; //Inclusive 
	int max = 9; //Exclusive
	
 	int probability = random(min,max);
 	return probability;
}

