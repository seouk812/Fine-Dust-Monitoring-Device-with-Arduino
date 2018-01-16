#include <LiquidCrystal.h> 				// LCD Display 이용하기 위한 header file
#include <SoftwareSerial.h> 				// Pm1001 센서와 Serial 통신하기 위한 header file
SoftwareSerial mySerial(8,9); 				// RX 8번(수신) TX9번 (송신)
unsigned char Send_data[4]={0x11, 0x01, 0x01, 0xED};	// 먼지센서로 보내는 값 (측정 명령)
unsigned char Receive_Buff[16];				// data buffer
unsigned long PCS; 					//농도 저장 변수
float ug;
unsigned char recv_cnt=0;

const int rs=12, en=11, d4=5, d5=4, d6=3, d7=2;		//LCD Display 출력을 위한 핀 번호 입력
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);			//객체 생성
void Send_CMD(void)	// command
{
	unsigned char i;
	for (i = 0; i < 4; i++){
		mySerial.write(Send_data[i]);
		delay(1);
	}
}
unsigned char Checksum_cal(void)	// checksum
{
	unsigned char count, SUM=0;
	for (count = 0; count < 15; count++)
		SUM+=Receive_Buff[count];
	return 256 – SUM;
}
void setup()
{
	pinMode(8, INPUT);
	pinMode(9, OUTPUT);
	Serial.begin(9600);
	while (!Serial);
	mySerial.begin(9600);
	while (!mySerial);
	lcd.begin(16, 2);
}
void loop()
{
	lcd.setCursor(0,0);			// LCD 커서 위치 맨위
	Send__CMD();			// Send Read Command
	while (1)
	{
		if (mySerial.availbable())	// 시리얼 통신이 가능한지 확인
		{
			Receive_Buff[recv_cnt++] = mySerial.read();
			if (recv_cnt == 16)
			{
				recv_cnt=0;
				break;
			}
		}
	}

	if (Checksum_cal() == Receive_Buff[15])	// CS 확인을 통해 통신 에러 없으면
	{
		PCS=(unsigned long)Receive_Buff[3] << 24 | (unsigned long)Receive_Buff[4] << 16
		       | (unsigned long)Receive_Buff[5] << 8 | (unsigned long)Receive_Buff[6];
		ug = (float)PCS*3528 / 100000;	// PCS 농도를  ㎍단위로 변환

		lcd.print(“Dust(ug) : ”);		// 먼지 값 출력
		lcd.print(ug);
		
		lcd.setCursor(0,1);			// 커서 위치 아래로
		lcd.print(“Status : ”);
		if (ug <= 30)			// 조건문을 통해 상태 출력
			lcd.print(“Good!”);
		else if (ug <= 80)
			lcd.print(“Not bad!”);
		else if (ug <=150)
			lcd.print(“Bad!”);
		else
			lcd.print(“Very bad!”);
	}
	else
		Serial.write(“CHECKSUM Error”);
	delay(1000);				// 1초(1000㎳)동안 대기
	lcd.clear();
}
