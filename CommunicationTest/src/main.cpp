#include <Arduino.h>
#include <NRF24.h>

#define INFOSTAT 0
#define INFONODE 1
#define INFOSTAT_DEBUG 0
#define INFONODE_DEBUG 0

String StartTime;

NRF24 nrf24(8,9);

uint16_t Interval = 0;

#if INFONODE
uint8_t NodeFlag = -1;
#endif

#if INFOSTAT 
uint8_t StationFlag = 0;
#endif

int Packet_Send(NRF24 NrfName, const char * Pipe, uint8_t * Packet){
    if (!NrfName.setTransmitAddress((uint8_t*)Pipe, 5))
        return -2;

    if (!NrfName.send(Packet, 32, true)) // NOACK, 110 microsecs
        return 0;

    if (NrfName.waitPacketSent())
        return -1;
}

uint8_t buf[32];

uint8_t * Packet_Receive(NRF24 NrfName, uint8_t Payload){
    uint8_t len = Payload;

    NrfName.waitAvailable();
    if (NrfName.recv(buf, &len)) // 140 microsecs
    {
        for(int i = 0; i < 32; i++)
            Serial.print((char)buf[i]);
        Serial.println();
    }
    return buf;
}

void SetReceive(NRF24 nrf){
    if (!nrf.init())
        Serial.println("NRF24_1 init failed");
    if (!nrf.setChannel(115))
        Serial.println("NRF24_1 setChannel failed");
    if (!nrf.setThisAddress((uint8_t*)"TX_01", 5))
        Serial.println("NRF24_1 setThisAddress failed");
    if (!nrf.setPayloadSize(32))
        Serial.println("NRF24_1 setPayloadSize failed");
    if (!nrf.setRF(NRF24::NRF24DataRate2Mbps, NRF24::NRF24TransmitPower0dBm))
        Serial.println("NRF24_1 setRF failed");    
    if (!nrf.powerUpRx())
        Serial.println("NRF24_1 powerOnRx failed");    
    Serial.println("SetReceive The End");
}

void SetTransmit(NRF24 nrf){
    if (!nrf.init())
        Serial.println("NRF24 init failed");
    if (!nrf.setChannel(115))
        Serial.println("setChannel failed");
    if (!nrf.setPayloadSize(32))
        Serial.println("setPayloadSize failed");
    if (!nrf.setRF(NRF24::NRF24DataRate2Mbps, NRF24::NRF24TransmitPower0dBm))
        Serial.println("setRF failed");    
    nrf.spiWriteRegister(NRF24_REG_1D_FEATURE, NRF24_EN_DYN_ACK);
    Serial.println("SetTransmit The End");
}

void setup() {
    // put your setup code here, to run once:
    Serial.begin(9600);
    while(!Serial) ;
#if INFOSTAT_DEBUG
    pinMode(5,OUTPUT);
    
    SetTransmit(nrf24);
#endif
#if INFONODE_DEBUG
    pinMode(5,OUTPUT);
    
    SetReceive(nrf24);
#endif
#if INFOSTAT
    pinMode(5,OUTPUT);
    
    SetTransmit(nrf24);
#endif

#if INFONODE
    pinMode(5,OUTPUT);
    
    SetReceive(nrf24);
#endif
}

void loop() {
    // put your main code here, to run repeatedly:
    uint8_t _buf[32];
    int PacketCount = 0;
    
#if INFOSTAT
    if(Serial.available() > 0){                 // 버퍼가 찼는지 확인
        
        byte str[32];                           // 버퍼에서 불러 온 값을 저장 할 배열
        Serial.readBytes(str,32);               // 버퍼값 불러오기
        
        digitalWrite(5,LOW);
        
        String result = String((char*)str);     // 배열을 문자열 형태로 변형 (startsWith 함수 사용을 위해)
        Serial.println(result);
        if(result.startsWith("S")){             // 문자열이 S로 시작하는지 확인 ==> 처음부터 START를 찾으니 못 찾는 경우가 종종 있었다.
            if(result.startsWith("START")){     // 문자열이 START로 시작하는지 확인
                Serial.println("rec");
                char numStr[4];                 // 숫자만 저장할 배열

                for(int i =0; i < 4; i++)       // 문자열에서 숫자부분 만 빼내서 저장
                    numStr[i] = result[8 + i];  // 0 ~ 11, START : 0001 
                
                Interval = atoi(numStr);        // 숫자 배열을 숫자로 변경하여 인터벌로 지정

                digitalWrite(5,LOW);

                Serial.print("OK : ");          // 응답
                Serial.println(Interval);         
            }        
        }
        
        else if(result[0] == '2' && result[1] == '0'){
            StartTime = result.substring(0,22);
            Serial.print("OK : ");              // 응답
            Serial.println(StartTime); 

            String str = String(StationFlag);
            Serial.print("before : "); Serial.println(str);   
            
            StationFlag = 1;
            str = String(StationFlag);
            Serial.print("after : "); Serial.println(str);   
        }
        Serial.flush();
    }
    
    if(StationFlag == 1)
    {
        Serial.println("Start Sensing");
        while(1)
        {
            SetTransmit(nrf24);

            char packet[] = "START : ";
            sprintf((char*)_buf,"%s%04d", packet, Interval);
            Serial.println((char*)_buf);

            Serial.println("AAAAAA send start");  

            for(int i = 0; i<5;i++)
            {
                Packet_Send(nrf24,"TX_01",_buf);
                delay(100);
            }
            Serial.println("AAAAAA send end");            
            SetReceive(nrf24);

            String NRFResult = (char *)Packet_Receive(nrf24,32);
            if(NRFResult.startsWith("O")){          // 문자열이 O로 시작하는지 확인
                if(NRFResult.startsWith("OK")){     // 문자열이 OK로 시작하는지 확인
                    digitalWrite(5,HIGH);           //통신시작을 알리기 위해 LED HIGH
                    Serial.print("SENSOR TEST START : ");          // 응답
                    Serial.println(Interval);  
                     
                    break;     
                }        
            }
        }

        //NODE 데이터 RECEIVE 시작

        while(1){    
            String NRFResult = (char *)Packet_Receive(nrf24,32);
            if(NRFResult.startsWith("STOP")){ // STOP이면 시리얼에 STOP출력 후 LED BLINK
                Serial.println(NRFResult); 
                for(int i = 0; i < 5; i++)
                {
                    digitalWrite(5,LOW);
                    delay(500);
                    digitalWrite(5,HIGH);
                    delay(500);
                }
                StationFlag=0;
                break;
            }
            else if(NRFResult.startsWith("A")){
                Serial.println(NRFResult);
            }
        }
    }
#endif

#if INFONODE
    while(1)
    {
        String NRFResult = (char *)Packet_Receive(nrf24,32);
        Serial.println(NRFResult);
        if(NRFResult.startsWith("S")){             // 문자열이 S로 시작하는지 확인 ==> 처음부터 START를 찾으니 못 찾는 경우가 종종 있었다.
            if(NRFResult.startsWith("START")){     // 문자열이 START로 시작하는지 확인
                char numStr[4];                 // 숫자만 저장할 배열

                for(int i =0; i < 4; i++)       // 문자열에서 숫자부분 만 빼내서 저장
                    numStr[i] = NRFResult[8 + i];  // 0 ~ 11, START : 0001 
                
                Interval = atoi(numStr);        // 숫자 배열을 숫자로 변경하여 인터벌로 지정

                digitalWrite(5,LOW);

                Serial.print("OK : ");          // 응답
                Serial.println(Interval);   

                SetTransmit(nrf24);
Serial.println("AAAAAA send start");  
                for(int i = 0; i<5;i++)
                {
                    Packet_Send(nrf24,"TX_01",(uint8_t *)"OK"); //STATION에 STOP문자열 전송 후 프로그램 종료
                    delay(100);
                }

                Serial.println("AAAAAA send end");  

                delay(1000);

                digitalWrite(5,HIGH); //통신시작을 알리기 위해 LED HIGH

                break;     
            }        
        }
    }

    while(1){
        char numStr[4];
        sprintf(numStr,"%04d",PacketCount);
        String packet = "AAAAAAAAAAAAAAAAAAAAAAAAAAAA" + String(numStr);
        Serial.println(packet);
        packet.toCharArray((char *)_buf,32);

        Packet_Send(nrf24,"TX_01",_buf);

        PacketCount++;

        delay(Interval);
        
        if(PacketCount == 1000) //1000번 전송했으면
        {
            sprintf((char *)_buf, "STOP"); 
            Packet_Send(nrf24,"TX_01",_buf); //STATION에 STOP문자열 전송 후 LED BLINK

            for(int i = 0; i < 5; i++)
            {
                digitalWrite(7,LOW);
                delay(500);
                digitalWrite(7,HIGH);
                delay(500);
            }

            break;
            
        }
    }
#endif

#if INFOSTAT_DEBUG
    uint8_t __buf[] = "NrfTest String Made in infoboss";
    if(!Packet_Send(nrf24,"TX_01",__buf)) //STATION에 STOP문자열 전송 후 프로그램 종료
        Serial.println("OK");
    else
        Serial.println("NOT OK");
    delay(100);
#endif
#if INFONODE_DEBUG
    String NRFResult = (char *)Packet_Receive(nrf24,32);
    Serial.println(NRFResult);
#endif
}
