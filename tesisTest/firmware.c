#include <stdint.h>
#include <stdbool.h>
#include <math.h> 

// a pointer to this is a null pointer, but the compiler does not
// know that because "sram" is a linker symbol from sections.lds.
extern uint32_t sram;

#define reg_spictrl (*(volatile uint32_t *)0x02000000)
#define reg_uart_clkdiv (*(volatile uint32_t *)0x02000004)
#define reg_uart_data (*(volatile uint32_t *)0x02000008)
#define clock (*(volatile uint32_t *)0x03000200)
#define pin_test (*(volatile uint32_t *)0x03000900)

extern uint32_t _sidata, _sdata, _edata, _sbss, _ebss, _heap_start;

uint32_t set_irq_mask(uint32_t mask);
asm(
    ".global set_irq_mask\n"
    "set_irq_mask:\n"
    ".word 0x0605650b\n"
    "ret\n");
/////////////////
void putchar(char c)
{
	if (c == '\n')
		putchar('\r');
	reg_uart_data = c;
}

void print(const char *p)
{
	while (*p)
		putchar(*(p++));
}

void print_hex(uint32_t v, int digits)
{
	for (int i = 7; i >= 0; i--)
	{
		char c = "0123456789abcdef"[(v >> (4 * i)) & 15];
		if (c == '0' && i >= digits)
			continue;
		putchar(c);
		digits = i;
	}
}

void print_dec(uint32_t v)
{
	if (v >= 100)
	{
		print(">=100");
		return;
	}

	if (v >= 90)
	{
		putchar('9');
		v -= 90;
	}
	else if (v >= 80)
	{
		putchar('8');
		v -= 80;
	}
	else if (v >= 70)
	{
		putchar('7');
		v -= 70;
	}
	else if (v >= 60)
	{
		putchar('6');
		v -= 60;
	}
	else if (v >= 50)
	{
		putchar('5');
		v -= 50;
	}
	else if (v >= 40)
	{
		putchar('4');
		v -= 40;
	}
	else if (v >= 30)
	{
		putchar('3');
		v -= 30;
	}
	else if (v >= 20)
	{
		putchar('2');
		v -= 20;
	}
	else if (v >= 10)
	{
		putchar('1');
		v -= 10;
	}

	if (v >= 9)
	{
		putchar('9');
		v -= 9;
	}
	else if (v >= 8)
	{
		putchar('8');
		v -= 8;
	}
	else if (v >= 7)
	{
		putchar('7');
		v -= 7;
	}
	else if (v >= 6)
	{
		putchar('6');
		v -= 6;
	}
	else if (v >= 5)
	{
		putchar('5');
		v -= 5;
	}
	else if (v >= 4)
	{
		putchar('4');
		v -= 4;
	}
	else if (v >= 3)
	{
		putchar('3');
		v -= 3;
	}
	else if (v >= 2)
	{
		putchar('2');
		v -= 2;
	}
	else if (v >= 1)
	{
		putchar('1');
		v -= 1;
	}
	else
		putchar('0');
}

void reverse(char *str, int len)
{
	int i = 0, j = len - 1, temp;
	while (i < j)
	{
		temp = str[i];
		str[i] = str[j];
		str[j] = temp;
		i++;
		j--;
	}
}

// Converts a given integer x to string str[].  d is the number
// of digits required in output. If d is more than the number
// of digits in x, then 0s are added at the beginning.
int intToStr(int x, char str[], int d, int offset)
{
	int i = 0;
	do
	{
		str[i++] = (x % 10) + '0';
		x = x / 10;
	}while (x);

	// If number of digits required is more, then
	// add 0s at the beginning
	while (i < d)
		str[i++] = '0';

	if(offset>0){
		str[i++]='-';
	}
	reverse(str, i);
	str[i] = '\0';
	return i;
}
// --------------------------------------------------------
// Converts a floating point number to string.
void ftoa(float n2, char *res)
{
	int extra=0;
	float n=n2;
	if(n<0){
		extra=1;
		n=-n2;
	}
	// Extract integer part
	int ipart = (int)n;

	// Extract floating part
	float fpart = n - (float)ipart;

	// convert integer part to string
	int i = intToStr(ipart, res, 0,extra);

	// check for display option after point
	res[i] = '.'; // add dot

	// Get the value of fraction part upto given no.
	// of points after dot. The third parameter is needed
	// to handle cases like 233.007
	fpart = fpart * pow(10, 4);

	intToStr((int)fpart, res + i + 1, 4,0);
}

void printf(float n)
{
	char res[20];
	intToStr(n, res, 0,0);
	ftoa(n, res);
	print(res);
	print("\n");
}

/*///////////////////////
Car code
////////////////////////*/

/* -------------------------------
  Other parameters
-----------------------------------*/
#define pointReachedThreshold 0.05
int timeToNewPoint=0;

bool finish = false;

bool countingTimeToEnd = false;
int timeToEnd = 0;


int timeControl=0;

int arr[200];

/* -------------------------------
  LEDS
-----------------------------------*/

/* -------------------------------
  PWM
-----------------------------------*/
#define pinPwmIzqF (*(volatile uint32_t *)0x03000500)
#define pinPwmIzqB (*(volatile uint32_t *)0x03000600)
#define pinPwmDerF (*(volatile uint32_t *)0x03000700)
#define pinPwmDerB (*(volatile uint32_t *)0x03000800)
#define maxPWM 150

int pwmValIzq = 0;
int pwmValDer = 0;

/* -------------------------------
  Encoders
-----------------------------------*/


//Counting variables
#define encoderCountIzq (*(volatile uint32_t *)0x03000300)
#define encoderCountDer (*(volatile uint32_t *)0x03000400)
int previousCountIzq = 0;
int previousCountDer = 0;

int timeCountSpeedIzq=0;
int timeCountSpeedDer=0;

void readEncoders()
{
  if (encoderCountDer != 0)
  {
    previousCountDer += encoderCountDer;
    encoderCountDer = 0;
  }
  if (encoderCountIzq != 0)
  {
    previousCountIzq += encoderCountIzq;
    encoderCountIzq = 0;
  }
}

/* -------------------------------
  Control
-----------------------------------*/
#define kr 2
#define ka  1
#define kb 0.04

float vRefDer = 0;
float vRefIzq = 0;

#define errorsLength 30

float errorIzq = 0;
float prevErrIzq = 0;
float errorSignalIzq = 0;

#define kpIzq  10
#define kiIzq 40
#define kdIzq  1

float errorDer = 0;
float prevErrDer = 0;
float errorSignalDer = 0;

#define kpDer  10
#define kiDer 40
#define kdDer 1

float integralErrorIzq;
float integralErrorDer;

long errorTime = 0;

/* -------------------------------
  Position
-----------------------------------*/

float curX = 0;
float curY = 0;
float curTheta = 0;

float v=0;
float w=0;

long timePassedDer = 0;
long timePassedIzq = 0;
float speedIzq = 0;
float speedDer = 0;
float dsIzq = 0;
float dsDer = 0;
float ds = 0;
float dTheta = 0;

float rho = 0;
float alpha = 0;
float beta = 0;
/* -------------------------------
  Car parameters
-----------------------------------*/
#define b  0.2
#define l  0.1
#define wheelRadius  0.034

int micros(){
	return clock;
}

/* -------------------------------
  Gradient function
-----------------------------------*/

float grad[2] = {0,0};
//Function = (x-2)^2+y^2 Le faltan -7
//3d plot (x-2)^2+y^2+10*e^(-((x-0.3)^2+(y-0.5)^2)*2)+12*e^(-((x-1.2)^2+(y+0.7)^2)*2) from -1 to 3
float gradX()
{
  //return 2*(curX+1);
  return 4*(curX-2) -100*(curX)*exp(-5*(pow(curX,2) + pow(curY - 0.7,2))) -120*(curX)*exp(-5*(pow(curX,2) + pow(curY + 1,2))) -120*(curX - 1.2)*exp(-5*(pow(curX - 1.2,2) + pow(curY + 1,2)));
}

float gradY()
{
  //return 2*(curY);
  return 4*(curY )  -100*(curY - 0.7)*exp(-5*(pow(curX,2) + pow(curY - 0.7,2))) -120*(curY +1)*exp(-5*(pow(curX,2) + pow(curY + 1,2))) -120*(curY + 1)*exp(-5*(pow(curX - 1.2,2) + pow(curY + 1,2)));
}

#define finalX 2
#define finalY 0

bool reachedNewPoint = true;
float newX = 0;
float newY = 0;

/* -------------------------------
  Odometry
-----------------------------------*/
void updateWheelsMovement()
{
  if (previousCountIzq > 10 || previousCountIzq < -10)
  {
    speedIzq = 2*(float)previousCountIzq * M_PI * wheelRadius*1000000 / (442*(micros()-timeCountSpeedIzq));
    timeCountSpeedIzq=micros();
    //Serial.print("izq count ");
    //Serial.println(previousCountIzq);
    //Serial.print("izq speed ");
    //Serial.println(speedIzq);
    previousCountIzq = 0;
  }
  else if(micros()-timeCountSpeedIzq>100000){
    speedIzq=0;
    previousCountIzq = 0;
    timeCountSpeedIzq=micros();
  }
  if (previousCountDer > 10 || previousCountDer < -10)
  {
    speedDer = 2*(float)previousCountDer* M_PI * wheelRadius*1000000 / (442*(micros()-timeCountSpeedDer));
    timeCountSpeedDer=micros();
    //Serial.print("der count ");
    //Serial.println(previousCountDer);
    //Serial.print("der speed ");
    //Serial.println(speedDer);
    previousCountDer = 0;
  }
  else if(micros()-timeCountSpeedDer>100000){
    speedDer=0;
    previousCountDer = 0;
    timeCountSpeedDer=micros();
  }
}

void calcNewPosition()
{
  long timeBetweenIzq = micros() - timePassedIzq;
  timePassedIzq = micros();
  dsIzq = timeBetweenIzq * speedIzq / 1000000;

  long timeBetweenDer = micros() - timePassedDer;
  timePassedDer = micros();
  dsDer = timeBetweenDer * speedDer / 1000000;

  ds = (dsIzq + dsDer) / 2;
  dTheta = (dsDer - dsIzq) / b;

  curX += ds * cos(curTheta + dTheta / 2);
  curY += ds * sin(curTheta + dTheta / 2);
  curTheta += dTheta;

  curTheta=curTheta>2*M_PI?curTheta-2*M_PI:curTheta;
  curTheta=curTheta<-2*M_PI?curTheta+2*M_PI:curTheta;
}

void odometry()
{
  updateWheelsMovement();
  calcNewPosition();
}

void calcNewPoint()
{
  newX = grad[0];
  newY = grad[1];
  float norm = sqrt((grad[0] * grad[0]) + (grad[1] * grad[1]));
  if (norm > 0.0001)
  {
    newX = newX / norm;
    newY = newY / norm;
  }

  newX *= 0.1;
  newY *= 0.1;



  newX+=curX;
  newY+=curY;
}

void calcControlVariables()
{
  float dX = newX - curX;
  float dY = newY - curY;

  rho = sqrt((dX * dX) + (dY * dY));

  if (rho < pointReachedThreshold || micros()-timeToNewPoint>1000000)
  {
    timeToNewPoint=micros();
    reachedNewPoint = true;
    return;
  }
  alpha = atan2(dY, dX) - curTheta;
  while (alpha > M_PI)
  {
    alpha -= 2 * M_PI;
  }
  while (alpha <= -M_PI)
  {
    alpha += 2 * M_PI;
  }
  beta = -alpha - curTheta;
  while (beta > M_PI)
  {
    beta -= 2 * M_PI;
  }
  while (beta <= -M_PI)
  {
    beta += 2 * M_PI;
  }
}

void calcRefVelocities()
{
  v = kr * rho;
  w = ka * alpha + kb * beta;

  vRefDer = (v +l * w);
  vRefIzq = v - l * w;

  if(vRefIzq>0.3) vRefIzq=0.3;
  if(vRefIzq<-0.3) vRefIzq=-0.3;

  if(vRefDer>0.3) vRefDer=0.3;
  if(vRefDer<-0.3) vRefDer=-0.3;


  //printf(vRefDer);

}

void controlOdometry()
{
  calcControlVariables();
  if (reachedNewPoint)
    return;
  calcRefVelocities();
}

void control()
{
  
  //printf(curY);
  //print("a");
  errorIzq = vRefIzq*1000 - speedIzq*1000;
  errorDer = vRefDer*1000 - speedDer*1000;

  integralErrorIzq+= errorIzq/10000;

  integralErrorDer+= errorDer/10000;
  float errorIzqDerivative = (errorIzq - prevErrIzq);
  float errorDerDerivative = (errorDer - prevErrDer);
  prevErrIzq = errorIzq;
  prevErrDer = errorDer;
  errorSignalIzq = kpIzq * errorIzq + kiIzq * integralErrorIzq + kdIzq * errorIzqDerivative;
  errorSignalDer = kpDer * errorDer + kiDer * integralErrorDer + kdDer * errorDerDerivative;

  if (errorSignalIzq < 0.1 && errorSignalIzq > -0.1)
  {
    errorSignalIzq = 0;
  }
  if (errorSignalDer < 0.1 && errorSignalDer > -0.1)
  {
    errorSignalDer = 0;
  }

  pwmValIzq = errorSignalIzq;
  pwmValDer = errorSignalDer;




  
  
 /* if(micros()-timeC>300000){
    Serial.print("curX ");
    Serial.println(curX);
    Serial.print("curY ");
    Serial.println(curY);
    Serial.print("curTheta ");
    Serial.println(curTheta*180/M_PI,4);
    //Serial.print("dsIzq ");
    //Serial.println(dsIzq,7);
    //Serial.print("pwmValDer ");
    //Serial.println(pwmValDer,7);
    //Serial.print("Ñam ");
    //Serial.println(vRefIzq);
     //Serial.print("Ñam1.1 ");
    //Serial.println(speedIzq);
    //Serial.print("Ñam1.5 ");
    //Serial.println(integralErrorIzq);
    //Serial.print("Ñam1.6 ");
    //Serial.println(errorIzq);
    //Serial.print("Ñam1.7 ");
    //Serial.println(errorDerDerivative);
    //Serial.print("Ñam2 ");
    //Serial.println(errorSignalIzq);
    //Serial.print("Ñam3 ");
    //Serial.println(pwmValIzq);
    //Serial.print("Rho ");
    //Serial.println(rho);
    //Serial.print("Alpha ");
    //Serial.println(alpha*180/M_PI);
    //Serial.print("Beta ");
    //Serial.println(beta*180/M_PI);
    //Serial.print("New X ");
    //Serial.println(newX);
    //Serial.print("New Y ");
    //Serial.println(newY);
    Serial.print("GradX ");
    Serial.println(grad[0]);
    Serial.print("GradY ");
    Serial.println(grad[1]);
    //Serial.print("w ");
    //Serial.println(w,4);
    //Serial.print("v ");
    //Serial.println(v,4);
    //Serial.print("vRefDer ");
    //Serial.println(vRefDer,4);
    //Serial.print("vRefIzq ");
    //Serial.println(vRefIzq,4);
    
    
    timeC=micros();
  } */

}

void analogWrite(volatile uint32_t * direction, int number){
	(*direction)=number;
}

void moveCar()
{
  volatile uint32_t * curPinPwmIzq = pwmValIzq >= 0 ? ((volatile uint32_t *)0x03000500) : ((volatile uint32_t *)0x03000600);
  volatile uint32_t * curPinPWMOffIzq = pwmValIzq >= 0 ? ((volatile uint32_t *)0x03000600) : ((volatile uint32_t *)0x03000500);
  int curPWMValIzq = pwmValIzq >= 0 ? pwmValIzq : -pwmValIzq;

  if (vRefIzq == 0)
  {
    curPWMValIzq = 0;
  }

  curPWMValIzq = curPWMValIzq > maxPWM ? maxPWM : curPWMValIzq;

  volatile uint32_t * curPinPwmDer = pwmValDer >= 0 ? ((volatile uint32_t *)0x03000700) : ((volatile uint32_t *)0x03000800);
  volatile uint32_t * curPinPWMOffDer = pwmValDer >= 0 ? ((volatile uint32_t *)0x03000800) : ((volatile uint32_t *)0x03000700);
  int curPWMValDer = pwmValDer >= 0 ? pwmValDer : -pwmValDer;

  if (vRefDer == 0)
  {
    curPWMValDer = 0;
  }

  curPWMValDer = curPWMValDer > maxPWM ? maxPWM : curPWMValDer;

  //printf(curPWMValIzq);
  analogWrite(curPinPWMOffIzq, 0);
  analogWrite(curPinPwmIzq, (int) curPWMValIzq);

  analogWrite(curPinPWMOffDer, 0);
  analogWrite(curPinPwmDer, (int) curPWMValDer);

 /* if(micros()-timeC>10000){
    Serial.print("curX ");
    Serial.println(curX);
    Serial.print("curY ");
    Serial.println(curY);
    Serial.print("curTheta ");
    Serial.println(curTheta);
    timeC=micros();
    Serial.print("Ñam ");
    Serial.println(curPWMValIzq);
    Serial.print("Ñam2 ");
    Serial.println(curPinPwmIzq);
  }*/
}

float calcDistanceToEnd()
{
  float dX = finalX - curX;
  float dY = finalY - curY;
  float norm = sqrt((dX * dX) + (dY * dY));
  return norm;
}


void stopCar()
{
  volatile uint32_t * t =((volatile uint32_t *)0x03000800);
  analogWrite(t, 0);
  analogWrite(((volatile uint32_t *)0x03000700), 0);

  analogWrite(((volatile uint32_t *)0x03000600), 0);
  analogWrite(((volatile uint32_t *)0x03000500), 0);
}

void setup(){
    /*float clockA = clock;
    reg_leds = 0;*/
	integralErrorIzq = 200/kiIzq;
  integralErrorDer =200/kiDer;
	timeToNewPoint=micros();
	timeCountSpeedIzq=micros();
	timeCountSpeedDer=micros();
}
int cycles=0;
void loop(){
  print("a");
  if (finish)
    return;
  readEncoders();

  odometry();

  float trueGradX = -gradX();
  float trueGradY = -gradY();

  grad[0] = trueGradX;
  grad[1] = trueGradY;

  if (reachedNewPoint)
  {
    reachedNewPoint = false;
    calcNewPoint();
  }

  controlOdometry();

  control();
  moveCar();
  if (calcDistanceToEnd() < 0.15)
  {
    //digitalWrite(pinLedFinishing, HIGH);
    if (countingTimeToEnd == false)
    {
      countingTimeToEnd = true;
      //Serial.println("counting end");
    }
    else
    {
      finish = true;
      //digitalWrite(pinLedFinished, HIGH);
      stopCar();
     /* Serial.println("Finished");
      Serial.print("CurX ");
      Serial.println(curX);
      Serial.print("CurY ");
      Serial.println(curY);*/
    }
  }
  else
  {
    countingTimeToEnd = false;
    //digitalWrite(pinLedFinishing, LOW);
  }
}


void main()
{
	reg_uart_clkdiv = 139;

	set_irq_mask(0xff);

	// zero out .bss section
	for (uint32_t *dest = &_sbss; dest < &_ebss;)
	{
		*dest++ = 0;
	}

	// switch to dual IO mode
	reg_spictrl = (reg_spictrl & ~0x007F0000) | 0x00400000;

	print("\n");
	print("  ____  _          ____         ____\n");
	print(" |  _ \\(_) ___ ___/ ___|  ___  / ___|\n");
	print(" | |_) | |/ __/ _ \\___ \\ / _ \\| |\n");
	print(" |  __/| | (_| (_) |__) | (_) | |___\n");
	print(" |_|   |_|\\___\\___/____/ \\___/ \\____|\n");

    setup();

    while (1)
    {
        loop();
    }
}