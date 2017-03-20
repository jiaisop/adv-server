#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <netinet/in.h>
#include <arpa/inet.h>


// Function Prototype
void moveStop();
void moveForward();
void moveBackward();
void moveLeft();
void moveRight();
void liftUp();
void liftDown();
void liftStop();
void vehicleHandler(char);
void netActionHandler(int, char*, int, int*, int*, int*);
enum State {start, autoCtrl,wait, stopState, goForward, turnLeft, turnRight, preLTurn, preRTurn, LBranchTurn, 
	  RBranchTurn, ForwardBfrTurn, prepathCnt,pathCntpuls, pre180, turn180, cameraCtrl, liftRise, liftDownState, Backward, lineEnd, jobDone} state;
// Function Prototype END

 
 // Define GPIO
const int mot1Pin1 = 16; 
const int mot1Pin2 = 12; 
const int mot2Pin1 = 21; 
const int mot2Pin2 = 20; 
const int mot3Pin1 = 13; 
const int mot3Pin2 = 6; 
const int mot4Pin1 = 26; 
const int mot4Pin2 = 19; 
const int IR_FrontM = 17;
const int IR_FrontL = 27;
const int IR_FrontR = 22;
const int IR_SideL = 10;
const int IR_SideR = 9;
const int LiftPin1 = 2; 
const int LiftPin2 = 3; 
const int LiftEnable = 4; 
// Define GPIO END 

//warehouse path decoder
int NA[] = {-1, 1}, AB[] = {-1,0,-1},BA[] = {1,0,1};
int NB[] = {1, -1}, AN[] = {-1,-1},  BN[] = {1,-1};
char tempChar = '0'; //Input from APP "NA" as test case
int TurnSignal[4];
int pathCnt = -1;
int jobDoneBool = 1;
//int * jobBoolDone = mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);

void Tick()
{
	switch (state) {
		
		
		case start:
		state = autoCtrl; // go to --> wait state after autoCtrl and manualCtrl is build
		break;
		
		case autoCtrl:
		if (tempChar=='1') {
			TurnSignal[0]=-1; 
			TurnSignal[1]=1; 
			TurnSignal[2]=-1;
			TurnSignal[3]=1; 
			state = wait;}
		else if (tempChar=='2') {
			TurnSignal[0]=1; 
			TurnSignal[1]=-1; 
			TurnSignal[2]=1;
			TurnSignal[3]=-1;  
			state = wait;}
		/*else if (tempChar=='3') {TurnSignal[0]=-1; TurnSignal[1]=1; TurnSignal[2]=0; state = wait;}
		else if (tempChar=='4') {TurnSignal[0]=-1; TurnSignal[1]=0; TurnSignal[2]=1; state = wait;}
		else if (tempChar=='5') {TurnSignal[0]=1; TurnSignal[1]=0; TurnSignal[2]=1; state = wait;}
		else if (tempChar=='6') {TurnSignal[0]=1; TurnSignal[1]=-1; TurnSignal[2]=0; state = wait;}*/
		break;
		
		case wait:
		if (digitalRead(IR_FrontM)==1&& digitalRead(IR_FrontL)==0 && digitalRead(IR_FrontR)==0 && digitalRead(IR_SideL)==0&&digitalRead(IR_SideR)==0){
			state = goForward;
		}
		else if(digitalRead(IR_FrontM)==0 && digitalRead(IR_FrontL)==1 && digitalRead(IR_FrontR)==0 && digitalRead(IR_SideL)==0&&digitalRead(IR_SideR)==0){
			state = turnLeft;
		}
		else if(digitalRead(IR_FrontM)==0 && digitalRead(IR_FrontR)==1 && digitalRead(IR_FrontL)==0&& digitalRead(IR_SideL)==0&&digitalRead(IR_SideR)==0){
			state = turnRight;
		}
		else{
			state = wait;
		}
		
		break;
		
		case stopState:
		if(pathCnt==1){
			state = lineEnd;
		}
		else if(pathCnt==3){
			state = jobDone;
		}	
		else if (digitalRead(IR_FrontM)==1 && digitalRead(IR_FrontL)==1 && digitalRead(IR_FrontR)==1&&digitalRead(IR_SideL)==1&&digitalRead(IR_SideL)==1){
			state = wait;
		}
		else if (digitalRead(IR_FrontM)==1&& digitalRead(IR_FrontL)==0 && digitalRead(IR_FrontR)==0&& digitalRead(IR_SideL)==0&&digitalRead(IR_SideR)==0){
			state = goForward;
		}
		else if(digitalRead(IR_FrontM)==0 && digitalRead(IR_FrontL)==1 && digitalRead(IR_FrontR)==0 && digitalRead(IR_SideL)==0&&digitalRead(IR_SideR)==0){
			state = turnLeft;
		}
		else if(digitalRead(IR_FrontM)==0 && digitalRead(IR_FrontR)==1 && digitalRead(IR_FrontL)==0&& digitalRead(IR_SideL)==0&&digitalRead(IR_SideR)==0){
			state = turnRight;
		}
		else if(digitalRead(IR_FrontM)==0 && digitalRead(IR_FrontL)==0 && digitalRead(IR_FrontR)==0&& digitalRead(IR_SideL)==0&&digitalRead(IR_SideR)==0){
			state = wait;
		}
		
		break;
		
		case goForward:
		if(digitalRead(IR_SideL)==1 && TurnSignal[pathCnt]==-1){
			state = ForwardBfrTurn;
		}
		else if(digitalRead(IR_SideR)==1 && TurnSignal[pathCnt]==1){
			state = ForwardBfrTurn;
		}
		else if(digitalRead(IR_FrontM)==1 && digitalRead(IR_FrontL)==1 && digitalRead(IR_FrontR)==1&& digitalRead(IR_SideL)==0&&digitalRead(IR_SideR)==0){
			state = prepathCnt;
		}
		else if (digitalRead(IR_FrontM)==1&& digitalRead(IR_FrontL)==0 && digitalRead(IR_FrontR)==0 && digitalRead(IR_SideL)==0&&digitalRead(IR_SideR)==0){
			state = goForward;
		}
		else if(digitalRead(IR_FrontM)==0 && digitalRead(IR_FrontL)==1 && digitalRead(IR_FrontR)==0 && digitalRead(IR_SideL)==0&&digitalRead(IR_SideR)==0){
			state = turnLeft;
		}
		else if(digitalRead(IR_FrontM)==0 && digitalRead(IR_FrontR)==1 && digitalRead(IR_FrontL)==0&& digitalRead(IR_SideL)==0&&digitalRead(IR_SideR)==0){
			state = turnRight;
		}
		else if(digitalRead(IR_FrontM)==0 && digitalRead(IR_FrontL)==0 && digitalRead(IR_FrontR)==0&& digitalRead(IR_SideL)==0&&digitalRead(IR_SideR)==0){
			state = stopState;
		}
		//else if (digitalRead(IR_FrontM)==1 && digitalRead(IR_FrontL)==1 && digitalRead(IR_FrontR)==1&&digitalRead(IR_SideL)==1&&digitalRead(IR_SideL)==1){
		//	state = stopState;
		//}
		break;
		
		case turnLeft:
		if(digitalRead(IR_SideL)==1 && TurnSignal[pathCnt]==-1){
			state = ForwardBfrTurn;
		}
		else if(digitalRead(IR_SideR)==1 && TurnSignal[pathCnt]==1){
			state = ForwardBfrTurn;
		}
		else if (digitalRead(IR_FrontM)==1&& digitalRead(IR_FrontL)==0 && digitalRead(IR_FrontR)==0 && digitalRead(IR_SideL)==0&&digitalRead(IR_SideR)==0){
			state = goForward;
		}
		else if(digitalRead(IR_FrontM)==0 && digitalRead(IR_FrontL)==1 && digitalRead(IR_FrontR)==0&& digitalRead(IR_SideL)==0&&digitalRead(IR_SideR)==0){
			state = turnRight;
		}
		else if(digitalRead(IR_FrontM)==0 && digitalRead(IR_FrontR)==1 && digitalRead(IR_FrontL)==0&& digitalRead(IR_SideL)==0&&digitalRead(IR_SideR)==0){
			state = turnLeft;
		}
		else if(digitalRead(IR_FrontM)==0 && digitalRead(IR_FrontL)==0 && digitalRead(IR_FrontR)==0&& digitalRead(IR_SideL)==0&&digitalRead(IR_SideR)==0){
			state = stopState;
		}
		//else if (digitalRead(IR_FrontM)==1 && digitalRead(IR_FrontL)==1 && digitalRead(IR_FrontR)==1&&digitalRead(IR_SideL)==1&&digitalRead(IR_SideL)==1){
		//	state = stopState;
		//}
		break;
		
		case turnRight:
		if(digitalRead(IR_SideL)==1 && TurnSignal[pathCnt]==-1){
			state = ForwardBfrTurn;
		}
		else if(digitalRead(IR_SideR)==1 && TurnSignal[pathCnt]==1){
			state = ForwardBfrTurn;
		}
		else if (digitalRead(IR_FrontM)==1&& digitalRead(IR_FrontL)==0 && digitalRead(IR_FrontR)==0 && digitalRead(IR_SideL)==0&&digitalRead(IR_SideR)==0){
			state = goForward;
		}
		else if(digitalRead(IR_FrontM)==0 && digitalRead(IR_FrontL)==1 && digitalRead(IR_FrontR)==0&& digitalRead(IR_SideL)==0&&digitalRead(IR_SideR)==0){
			state = turnRight;
		}
		else if(digitalRead(IR_FrontM)==0 && digitalRead(IR_FrontR)==1 && digitalRead(IR_FrontL)==0&& digitalRead(IR_SideL)==0&&digitalRead(IR_SideR)==0){
			state = turnLeft;
		}
		else if(digitalRead(IR_FrontM)==0 && digitalRead(IR_FrontL)==0 && digitalRead(IR_FrontR)==0&& digitalRead(IR_SideL)==0&&digitalRead(IR_SideR)==0){
			state = stopState;
		}
		//else if (digitalRead(IR_FrontM)==1 && digitalRead(IR_FrontL)==1 && digitalRead(IR_FrontR)==1&&digitalRead(IR_SideL)==1&&digitalRead(IR_SideL)==1){
		//	state = stopState;
		//}
		break;
		
		case preLTurn:
		delay(400);
		state = LBranchTurn;
		break;
		
		case preRTurn:
		delay(400);
		state = RBranchTurn;
		break;
		
		case LBranchTurn:
		//delay(400);
		if(digitalRead(IR_FrontM)==1&& digitalRead(IR_FrontL)==0 && digitalRead(IR_FrontR)==0 && digitalRead(IR_SideL)==0&&digitalRead(IR_SideR)==0){
			printf("LturnDone\n");
			state = goForward;
		}
		else{
			state = LBranchTurn;
		}
		break;
		
		case RBranchTurn:
		//delay(400);
		if(digitalRead(IR_FrontM)==1&& digitalRead(IR_FrontL)==0 && digitalRead(IR_FrontR)==0 && digitalRead(IR_SideL)==0&&digitalRead(IR_SideR)==0){
			printf("RturnDone\n");
			state = goForward;
		}
		else{
			state = RBranchTurn;
		}
		break;
		
		case prepathCnt:
		if(digitalRead(IR_FrontM)==1 && digitalRead(IR_FrontL)==1 && digitalRead(IR_FrontR)==1&&digitalRead(IR_SideL)==0&&digitalRead(IR_SideR)==0){
			state = prepathCnt;
		}
		else if(digitalRead(IR_SideL)==1||digitalRead(IR_SideR)==1){
			state = pathCntpuls;
		}
		break;
		
		case pathCntpuls:
		state=goForward;
		break;
		
		case ForwardBfrTurn:
		delay(200);
	    if(TurnSignal[pathCnt]==-1){state=preLTurn;}
	    else if(TurnSignal[pathCnt]==1){state=preRTurn;}
		break;
		
		case pre180:
		delay(400);
		state = turn180;
		break;
		
		case turn180:
        //delay(400);
		if(digitalRead(IR_FrontM)==1&& digitalRead(IR_FrontL)==0 && digitalRead(IR_FrontR)==0 && digitalRead(IR_SideL)==0&&digitalRead(IR_SideR)==0){
			state = goForward;
		}
		else{
			state = turn180;
		}
		break;
		
		case lineEnd:
		delay(1500);
		state = liftDownState;
		break;
		
		case liftRise:
		break;
		
		case liftDownState:
		delay(9000);
		state = Backward;
		break;
		
		case Backward:
		delay(1500);
		state = turn180;
		break;	
		
		case jobDone:
		printf("DONE\n");
		state = start;
		break;
		
		default:
		state = goForward;
		break;
	}
	
	switch (state) {
		case start:
		moveStop();
		break;

		case autoCtrl:
		break;
		
		case wait:
		break;
		
		case liftRise:
		break;		

		case lineEnd:
		moveForward();
		break;

		case liftDownState:
		moveStop();
		liftDown();
		break;
		
		case Backward:
		liftStop();
		moveBackward();
		break;
		
		case pre180:
		moveLeft();
		break;
		
		case turn180:
		moveLeft();
		break;
		
		case jobDone:
		pathCnt=-1;
		jobDoneBool = 1;
		printf("Bool = 1 \n");
		moveStop();
		break;
		
		case stopState:
		//pathCnt=0;
		moveStop();
		break;
		
		case goForward:
		moveForward();
		//printf("w\n");
		break;
		
		case turnLeft:
		moveRight();
		//printf("Left\n");
		break;
		
		case turnRight:
		moveLeft();
		//printf("Right\n");
		break;
		
		case preLTurn:
		moveLeft();
		break;
		
		case preRTurn:
		moveRight();
		break;
		
		case LBranchTurn:
		moveLeft();
		//printf("Left\n");
		break;
		
		case RBranchTurn:
		moveRight();
		//printf("Right\n");
		break;
		
		case ForwardBfrTurn:
		moveForward();
		printf("***\n");
		break;
		
		case prepathCnt:
		moveForward();
		break;
		
		case pathCntpuls:
		pathCnt++;
		printf("%d %d\n",pathCnt,TurnSignal[pathCnt]);
		break;
		
		default:
		break;
    }
}

int main(void)
{
	// GPIO Setup
    wiringPiSetupGpio();

    pinMode(mot1Pin1, OUTPUT); 
    pinMode(mot1Pin2, OUTPUT);  
    pinMode(mot2Pin1, OUTPUT); 
    pinMode(mot2Pin2, OUTPUT);  
    pinMode(mot3Pin1, OUTPUT); 
    pinMode(mot3Pin2, OUTPUT);  
    pinMode(mot4Pin1, OUTPUT); 
    pinMode(mot4Pin2, OUTPUT);  
    pinMode(LiftPin1, OUTPUT); 
    pinMode(LiftPin2, OUTPUT);
    pinMode(LiftEnable, OUTPUT);
    digitalWrite(LiftEnable, HIGH);
    // GPIO Setup END
    
    moveStop();
    
    int * cam_move = mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    int * line_only = mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    int * temptempChar = mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    *cam_move = 1;
    *line_only = 0;
    *temptempChar = 0;
    
    int primarySocket, transferSocket, pid;
	struct sockaddr_in serv_addr, cli_addr;
	socklen_t clilen;
	const int serverPort = 3000;
	primarySocket = socket(AF_INET, SOCK_STREAM, 0);
	if (primarySocket < 0) {
		perror("Can't open socket");
		return 1;
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(serverPort);
	if (bind(primarySocket, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("Can't bind");
		return 1;
	}
	if(listen(primarySocket, 5) < 0) {
		perror("Can't listen to socket");
		return 1;
	}
	printf("ADV Server is now online!\n\n");
    
    while (1) {
		clilen = sizeof(cli_addr);
		transferSocket = accept(primarySocket, (struct sockaddr *) &cli_addr, &clilen);
		char *opIP = inet_ntoa(cli_addr.sin_addr);
		int opPort = (int) ntohs(cli_addr.sin_port);
		printf("Connection From: %s:%d\n", opIP, opPort);
		if (transferSocket < 0) {
			perror("Can't Accept");
			return 1;
		}
		pid = fork();
		if (pid < 0) {
			perror("Can't Fork");
			return 1;
		}
		if (pid == 0) {
			close(primarySocket);
			while(1) {
				state = start;
				jobDoneBool = 0;
				while(*cam_move == 1 && *line_only == 0){
					netActionHandler(transferSocket, opIP, opPort, cam_move, line_only, temptempChar);
				}
				while(*cam_move == 0 && *line_only == 1 && jobDoneBool == 0){
					if(*temptempChar == 0){
						netActionHandler(transferSocket, opIP, opPort, cam_move, line_only, temptempChar);
					}
					Tick();
					//if(jobDoneBool == 1){
					//	*cam_move = 1;
					//	*line_only = 0;
					//}
				}
				close(transferSocket);
				printf("\%s:%d disconnected\n\n", opIP, opPort);
				exit(0);
			}
			close(transferSocket);
			printf("\%s:%d disconnected\n\n", opIP, opPort);
			exit(0);
		}
		else {
			close(transferSocket);
		}
	}
	close(primarySocket);
	return 0;
}


void moveStop()
{
	digitalWrite(mot1Pin1, LOW);
	digitalWrite(mot1Pin2, LOW);
	digitalWrite(mot2Pin1, LOW);
	digitalWrite(mot2Pin2, LOW);
	digitalWrite(mot3Pin1, LOW);
	digitalWrite(mot3Pin2, LOW);
	digitalWrite(mot4Pin1, LOW);
	digitalWrite(mot4Pin2, LOW);
}


void moveForward()
{
	digitalWrite(mot1Pin1, LOW);
	digitalWrite(mot1Pin2, HIGH);
	digitalWrite(mot2Pin1, HIGH);
	digitalWrite(mot2Pin2, LOW);
	digitalWrite(mot3Pin1, LOW);
	digitalWrite(mot3Pin2, HIGH);
	digitalWrite(mot4Pin1, HIGH);
	digitalWrite(mot4Pin2, LOW);
}


void moveBackward()
{
	digitalWrite(mot1Pin1, HIGH);
	digitalWrite(mot1Pin2, LOW);
	digitalWrite(mot2Pin1, LOW);
	digitalWrite(mot2Pin2, HIGH);
	digitalWrite(mot3Pin1, HIGH);
	digitalWrite(mot3Pin2, LOW);
	digitalWrite(mot4Pin1, LOW);
	digitalWrite(mot4Pin2, HIGH);
}


void moveLeft()
{
	digitalWrite(mot1Pin1, HIGH);
	digitalWrite(mot1Pin2, LOW);
	digitalWrite(mot2Pin1, HIGH);
	digitalWrite(mot2Pin2, LOW);
	digitalWrite(mot3Pin1, HIGH);
	digitalWrite(mot3Pin2, LOW);
	digitalWrite(mot4Pin1, HIGH);
	digitalWrite(mot4Pin2, LOW);
}


void moveRight()
{		
	digitalWrite(mot1Pin1, LOW);
	digitalWrite(mot1Pin2, HIGH);
	digitalWrite(mot2Pin1, LOW);
	digitalWrite(mot2Pin2, HIGH);
	digitalWrite(mot3Pin1, LOW);
	digitalWrite(mot3Pin2, HIGH);
	digitalWrite(mot4Pin1, LOW);
	digitalWrite(mot4Pin2, HIGH);
	
}

void liftUp()
{
	digitalWrite(LiftPin1, HIGH);
	digitalWrite(LiftPin2, LOW);
}

void liftDown()
{
	digitalWrite(LiftPin1, LOW);
	digitalWrite(LiftPin2, HIGH);
}

void liftStop()
{
	digitalWrite(LiftPin1, LOW);
	digitalWrite(LiftPin2, LOW);
}


void vehicleHandler(char userInput)
{
	// User Input Start
	
	if(userInput == 'h')
	{
		printf("Stop \n");
		moveStop();
	}
	else if(userInput == 'w')
	{
		printf("Forward \n");
		moveForward();
	}
	else if(userInput == 's')
	{
		printf("Backward \n");
		moveBackward();
	}
	else if(userInput == 'a')
	{
		printf("Left \n");
		moveLeft();
	}
	else if(userInput == 'd')
	{
		printf("Right \n");
		moveRight();
	}
	else if(userInput == 'q')
	{
		printf("Lift Up \n");
		liftUp();
	}
	else if(userInput == 'e')
	{
		printf("Lift Down \n");
		liftDown();
	}
	else if(userInput == ' ')
	{
		printf("Lift Stop \n");
		liftStop();
	}
	
	// User Input END
}


void netActionHandler(int childSock, char* childAddr, int childPort, int* cam_move, int* line_only, int* temptempChar)
{
	char buffer[256];
	char messageForClient[20];
	int requestStatus = 0;

	bzero(messageForClient, 20);
	bzero(buffer, 256);
	requestStatus = recv(childSock, buffer, 255, 0);
	if (requestStatus < 0) {
		perror("Can't read from client");
		close(childSock);
		exit(1);
	}
	else if (requestStatus == 0) {
	   printf("\%s:%d disconnected\n\n", childAddr, childPort);
	   close(childSock);
	   exit(1);
	}
	
	if (strlen(buffer) == 2) {
		if (buffer[0] == 'w') {
			strcpy(messageForClient, "Forward");
			printf("\nCommand from %s:%d\n", childAddr, childPort);
			printf("%s \nADV Status: ", messageForClient);
			vehicleHandler(buffer[0]);
		}
		else if(buffer[0] == 's') {
			strcpy(messageForClient, "Backward");
			printf("\nCommand from %s:%d\n", childAddr, childPort);
			printf("%s \nADV Status: ", messageForClient);
			vehicleHandler(buffer[0]);
		}
		else if(buffer[0] == 'a') {
			strcpy(messageForClient, "Left");
			printf("\nCommand from %s:%d\n", childAddr, childPort);
			printf("%s \nADV Status: ", messageForClient);
			vehicleHandler(buffer[0]);
		}
		else if(buffer[0] == 'd') {
			strcpy(messageForClient, "Right");
			printf("\nCommand from %s:%d\n", childAddr, childPort);
			printf("%s \nADV Status: ", messageForClient);
			vehicleHandler(buffer[0]);
		}
		else if(buffer[0] == 'h') {
			strcpy(messageForClient, "Stop");
			printf("\nCommand from %s:%d\n", childAddr, childPort);
			printf("%s \nADV Status: ", messageForClient);
			vehicleHandler(buffer[0]);
		}
		else if(buffer[0] == 'q') {
			strcpy(messageForClient, "Lift Up");
			printf("\nCommand from %s:%d\n", childAddr, childPort);
			printf("%s \nADV Status: ", messageForClient);
			vehicleHandler(buffer[0]);
		}
		else if(buffer[0] == 'e') {
			strcpy(messageForClient, "Lift Down");
			printf("\nCommand from %s:%d\n", childAddr, childPort);
			printf("%s \nADV Status: ", messageForClient);
			vehicleHandler(buffer[0]);
		}
		else if(buffer[0] == ' ') {
			strcpy(messageForClient, "Lift Stop");
			printf("\nCommand from %s:%d\n", childAddr, childPort);
			printf("%s \nADV Status: ", messageForClient);
			vehicleHandler(buffer[0]);
		}
		else if(buffer[0] == '1') {
			strcpy(messageForClient, "1");
			printf("\nCommand from %s:%d\n", childAddr, childPort);
			printf("%s \nADV Status: ", messageForClient);
			jobDoneBool = 0;
			printf("1 \n");
			*temptempChar = 1;
			tempChar = '1';
		}
		else if(buffer[0] == '2') {
			strcpy(messageForClient, "2");
			printf("\nCommand from %s:%d\n", childAddr, childPort);
			printf("%s \nADV Status: ", messageForClient);
			jobDoneBool = 0;
			printf("2 \n");
			*temptempChar = 2;
			tempChar = '2';
		}
		else if(buffer[0] == '3') {
			strcpy(messageForClient, "3");
			printf("\nCommand from %s:%d\n", childAddr, childPort);
			printf("%s \nADV Status: ", messageForClient);
			printf("3 \n");
			tempChar = '3';
		}
		else if(buffer[0] == '4') {
			strcpy(messageForClient, "4");
			printf("\nCommand from %s:%d\n", childAddr, childPort);
			printf("%s \nADV Status: ", messageForClient);
			printf("4 \n");
			tempChar = '4';
		}
		else if(buffer[0] == '5') {
			strcpy(messageForClient, "5");
			printf("\nCommand from %s:%d\n", childAddr, childPort);
			printf("%s \nADV Status: ", messageForClient);
			printf("5 \n");
			tempChar = '5';
		}
		else if(buffer[0] == '6') {
			strcpy(messageForClient, "6");
			printf("\nCommand from %s:%d\n", childAddr, childPort);
			printf("%s \nADV Status: ", messageForClient);
			printf("6 \n");
			tempChar = '6';
		}
		else {
			strcpy(messageForClient, "Try Again");
			printf("\nCommand from %s:%d\n", childAddr, childPort);
			printf("Invalid input: %s \n", buffer);
		}
	}
	else if (strlen(buffer) == 3) {
		if(buffer[0] == 'o') {
			if(buffer[1] == 'k'){
				strcpy(messageForClient, "Camera movement off");
				printf("\nCommand from %s:%d\n", childAddr, childPort);
				printf("%s \nADV Status: ", messageForClient);
				vehicleHandler('h');
				*cam_move = 0;
				*line_only = 1;
			}
			else {
				strcpy(messageForClient, "Try Again");
				printf("\nCommand from %s:%d\n", childAddr, childPort);
				printf("Invalid input: %s \n", buffer);
			}
		}
		else {
			strcpy(messageForClient, "Try Again");
			printf("\nCommand from %s:%d\n", childAddr, childPort);
			printf("Invalid input: %s \n", buffer);
		}
	}
	else {
		strcpy(messageForClient, "Try Again");
		printf("\nCommand from %s:%d\n", childAddr, childPort);
			printf("Invalid input: %s \n", buffer);
	}
	
	
	
	if (send(childSock, messageForClient, strlen(messageForClient), 0) < 0) {
	   perror("Can't write to client");
	   close(childSock);
	   exit(1);
	}
}
