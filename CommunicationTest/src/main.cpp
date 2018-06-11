#include <Arduino.h>
#include <NRF24.h>

#define INFOSTAT 1
#define INFONODE 0

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
    // Now send the samples NOACK (EN_DYN_ACK must be enabled first)
    // With 2Mbps, NOACK and 32 byte payload, can send about 1900 messages per sec
    if (!NrfName.setTransmitAddress((uint8_t*)Pipe, 5))
        return -2;
        //Serial.println("setTransmitAddress failed");
    // Send the data  
    if (!NrfName.send(Packet, sizeof(Packet), true)) // NOACK, 110 microsecs
        return 0;
        //Serial.println("send failed");  
    // Transmission takes about 300 microsecs, of which about 130microsecs is transmitter startup time
    // and 160 microsecs is transmit time for 32 bytes+8 bytes overhead @ 2Mbps
    if (!NrfName.waitPacketSent())
        return -1;
        //Serial.println("waitPacketSent failed");
}

uint8_t * Packet_Receive(NRF24 NrfName, uint8_t Payload){
    uint8_t buf[Payload];
    uint8_t len = sizeof(buf);

    NrfName.waitAvailable();
    if (NrfName.recv(buf, &len)) // 140 microsecs
    {
        for(int i = 0; i < 32; i++)
            Serial.print((char)buf[i]);
        Serial.println();
    }
    return buf;
}

void setup() {
    // put your setup code here, to run once:
    pinMode(7,OUTPUT);
    
    Serial.begin(9600);
    while (!Serial) ; // wait for serial port to connect. Needed for Leonardo only
    if (!nrf24.init())
        Serial.println("NRF24 init failed");
    if (!nrf24.setChannel(115))
        Serial.println("setChannel failed");
    if (!nrf24.setPayloadSize(32))
        Serial.println("setPayloadSize failed");
    if (!nrf24.setRF(NRF24::NRF24DataRate250kbps, NRF24::NRF24TransmitPower0dBm))
        Serial.println("setRF failed");    
    // Enable the EN_DYN_ACK feature so we can use noack
    nrf24.spiWriteRegister(NRF24_REG_1D_FEATURE, NRF24_EN_DYN_ACK);
    Serial.println("initialised");
}

void loop() {
    // put your main code here, to run repeatedly:
    uint8_t buf[32];
    uint16_t PacketCount = 0;

#pragma region 라즈베리파이간 통신   

    if(Serial.available() > 0){                 // 버퍼가 찼는지 확인
        byte str[32];                           // 버퍼에서 불러 온 값을 저장 할 배열
        Serial.readBytes(str,32);               // 버퍼값 불러오기
        
        digitalWrite(7,HIGH);                   
        
        String result = String((char*)str);     // 배열을 문자열 형태로 변형 (startsWith 함수 사용을 위해)
        
        if(result.startsWith("S")){             // 문자열이 S로 시작하는지 확인 ==> 처음부터 START를 찾으니 못 찾는 경우가 종종 있었다.
            if(result.startsWith("START")){     // 문자열이 START로 시작하는지 확인
                char numStr[4];                 // 숫자만 저장할 배열

                for(int i =0; i < 4; i++)       // 문자열에서 숫자부분 만 빼내서 저장
                    numStr[i] = result[8 + i];  // 0 ~ 11, START : 0001 
                
                Interval = atoi(numStr);        // 숫자 배열을 숫자로 변경하여 인터벌로 지정

                digitalWrite(7,LOW);

                Serial.print("OK : ");          // 응답
                Serial.println(Interval);         
            }        
        }
        
        else if(result[0] == '2' && result[1] == '0'){
            StartTime = result.substring(0,26);
            Serial.print("OK : ");              // 응답
            Serial.println(StartTime);    
            StationFlag = 1;
        }
    }

#pragma endregion

#pragma region 개별 구간

#if INFOSTAT
    if(StationFlag == 1){
        String packet = "START : ";
        sprintf((char*)buf,"%s%04d",packet,Interval);
        Serial.println((char*)buf);
        packet.toCharArray((char *)buf,32);
        while(1){
            Packet_Send(nrf24,"TX_01",buf);
            String NRFResult = (char *)Packet_Receive(nrf24,32);
            if(NRFResult.startsWith("OK")){
                StationFlag = 0;
            }
            else if(NRFResult.endsWith("1000")){
                Serial.println("STOP");
                break;
            }
            else if(NRFResult.startsWith("A")){
                Serial.println(NRFResult);
            }
        }
    }
#endif

#if INFONODE
    while(NodeFlag == 0){
        
        String packet = "AAAAAAAAAAAAAAAAAAAAAAAAAAAA";
        sprintf((char*)buf,"%s%04d",packet,PacketCount++);
        Serial.println((char*)buf);
        packet.toCharArray((char *)buf,32);

        Packet_Send(nrf24,"TX_01",buf);

        delay(Interval);
        
        if(PacketCount == 1000){
            NodeFlag = 1;
        }
    }
#endif

#pragma endregion

}
