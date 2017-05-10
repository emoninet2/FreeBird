#include "mbed.h"
#include "rtos.h"
#include "EthernetInterface.h"
#include "TCPSocket.h"
#include "TCPServer.h"

#include <stdio.h>
#include <string.h>

#include "NRF24L01p/NRF24L01p.h"

NRF24L01p Radio;

DigitalOut led1(LED1);

bool charIsDelim(char c, char *delim){

    char * pch = delim;
    pch=strchr(delim,c);
    
    if((pch-delim)>=0){
        return 1;
    }else{
        return 0;
    }
}


int parseString(char * s, int len,  char * delim, int maxTokens, char ** argv){
    int argc ;
       
    int i;
    bool removeWhiteSpace = 1;
    for(i=0;i<len;i++){

        if(  charIsDelim(s[i],delim)   && (removeWhiteSpace)  ){
            s[i] = 0;
        }

        if(s[i] == '"' ){
            s[i] = 0;
            removeWhiteSpace = !removeWhiteSpace;
        }
        
    }
    
    int start = 0;
    argc = 0;
    for(i=0;i<len;i++){
        if(s[i] != 0){
            start = i;
            argv[argc++] = &s[i];
            break;
        }
    }
    
    for(i=start+1;i<len;i++){
        if( (s[i] != 0) && (s[i-1] == 0)  ){
            argv[argc++] = &s[i];
            if(argc > maxTokens){
                break;
            }
        }
    }
    
    
    return argc;
}

void NRF24L01p_RadioReset(void){


    Radio.RadioConfig.DataReadyInterruptEnabled = 0;
    Radio.RadioConfig.DataSentInterruptFlagEnabled = 0;
    Radio.RadioConfig.MaxRetryInterruptFlagEnabled = 0;
    Radio.RadioConfig.Crc = NRF24L01p::CONFIG_CRC_16BIT;
    Radio.RadioConfig.AutoReTransmissionCount = 15;
    Radio.RadioConfig.AutoReTransmitDelayX250us = 15;
    Radio.RadioConfig.frequencyOffset = 2;
    Radio.RadioConfig.datarate = NRF24L01p::RF_SETUP_RF_DR_2MBPS;
    Radio.RadioConfig.RfPower = NRF24L01p::RF_SETUP_RF_PWR_0DBM;
    Radio.RadioConfig.PllLock = 0;
    Radio.RadioConfig.ContWaveEnabled = 0;
    Radio.RadioConfig.FeatureDynamicPayloadEnabled = 1;
    Radio.RadioConfig.FeaturePayloadWithAckEnabled = 1;
    Radio.RadioConfig.FeatureDynamicPayloadWithNoAckEnabled = 1;

    Radio.RxPipeConfig[0].address = 0xe7e7e7e7e7;
    Radio.RxPipeConfig[1].address = 0x6C616d7091;
    Radio.RxPipeConfig[2].address = 0x6C616d7092;
    Radio.RxPipeConfig[3].address = 0x6C616d7093;
    Radio.RxPipeConfig[4].address = 0x6C616d7094;
    Radio.RxPipeConfig[5].address = 0x6C616d7095;

    int i;
   
    for(i=0;i<6;i++){
        Radio.RxPipeConfig[i].PipeEnabled = 1;
        Radio.RxPipeConfig[i].autoAckEnabled = 1;
        Radio.RxPipeConfig[i].dynamicPayloadEnabled = 1;
    }

    Radio.Initialize();

}

void Nrf24l01p_CMD(char *cmdInOut){
	char *str = cmdInOut;
	char *tokenV[10];
	int n = parseString(str, strlen(str),  " -/,\r\n" , 10, tokenV);
	int i;

	for(i=0;i<n;i++){
		printf("\ttoken %d --> [%d] %s\r\n", i, strlen(tokenV[i]) ,tokenV[i]);
	}

	if(!(strcmp(tokenV[0],"config"))){
		printf("gonna do some configuration\r\n");
		sprintf(cmdInOut, "config\r\n");
	}
	if(!(strcmp(tokenV[0],"status"))){
		printf("gonna get some status\r\n");
		sprintf(cmdInOut, "%#x getting status\r\n", Radio.get_status());
	}
	else 	if(!(strcmp(tokenV[0],"GetPipeConfig"))){
		printf("gonna get some pipe configuration\r\n");
		NRF24L01p::pipe_t pipe;
		bool PipeEnabled;
		bool autoAckEnabled;
		uint64_t address;
		uint8_t MaxWidth;
		bool dynamicPayloadEnabled;
		sscanf(tokenV[1], "%d", &pipe);

		address = Radio.rxPipeAddress(pipe);
		PipeEnabled = Radio.RxOnPipe(pipe);
		autoAckEnabled = Radio.autoAckOnPipe(pipe);
		MaxWidth = Radio.rxPipeWidth(pipe);
		dynamicPayloadEnabled = Radio.dynamicPayloadOnPipe(pipe);


		sprintf(cmdInOut, "%d %#llx %d %d %d %d\r\n",pipe,address, PipeEnabled, autoAckEnabled,dynamicPayloadEnabled, MaxWidth);
	}
	else 	if(!(strcmp(tokenV[0],"SetPipeConfig"))){
		printf("gonna do some pipe configuration\r\n");
		NRF24L01p::pipe_t pipe;
		bool PipeEnabled;
		bool autoAckEnabled;
		uint64_t address;
		uint8_t MaxWidth;
		bool dynamicPayloadEnabled;
		
		sscanf(tokenV[1], "%d", &pipe);
		sscanf(tokenV[2], "%llx",&address);
		sscanf(tokenV[3], "%d", &PipeEnabled);
		sscanf(tokenV[4], "%d", &autoAckEnabled);
		sscanf(tokenV[5], "%d", &dynamicPayloadEnabled);
		sscanf(tokenV[6], "%d", &MaxWidth);
		
		Radio.set_RX_pipe_address(pipe,address);
		Radio.enable_rx_on_pipe(pipe, PipeEnabled);
		Radio.enable_auto_ack(pipe, autoAckEnabled);
		Radio.set_RX_pipe_width(pipe, MaxWidth);
		Radio.enable_dynamic_payload_pipe(pipe, dynamicPayloadEnabled);

		sprintf(cmdInOut, "%d %#llx %d %d %d %d\r\n",pipe,address, PipeEnabled, autoAckEnabled,dynamicPayloadEnabled, MaxWidth);
	}
	else 	if(!(strcmp(tokenV[0],"radio"))){
		printf("gonna use the radio\r\n");
		sprintf(cmdInOut, "radio command\r\n");
	}




	else 	if(!(strcmp(tokenV[0],"init"))){
		printf("gonna use the radio\r\n");
		sprintf(cmdInOut, "initializing radio\r\n");
		NRF24L01p_RadioReset();
	}
	else 	if(!(strcmp(tokenV[0],"mode"))){
		printf("gonna use the radio\r\n");
		sprintf(cmdInOut, "default return\r\n");
	}
	else 	if(!(strcmp(tokenV[0],"readable"))){
		printf("gonna use the radio\r\n");
		sprintf(cmdInOut, "%d\r\n", Radio.readable());
	}
	else 	if(!(strcmp(tokenV[0],"writable"))){
		printf("gonna use the radio\r\n");
		sprintf(cmdInOut, "%d\r\n", Radio.writable());
	}
	else 	if(!(strcmp(tokenV[0],"flagClear"))){
		
	}
	else 	if(!(strcmp(tokenV[0],"transmit"))){
		NRF24L01p::Payload_t payload;
		sscanf(tokenV[1], "%llx", &payload.address);
		sscanf(tokenV[2], "%d", &payload.length);
		memcpy(&payload.data, tokenV[3], payload.length);
		payload.data[payload.length] = '\0';
		sscanf(tokenV[4], "%d", &payload.retransmitCount);
		int error = Radio.TransmitPayload(&payload);
		sprintf(cmdInOut, "%d\r\n", error);

	}
	else 	if(!(strcmp(tokenV[0],"receive"))){
		if(Radio.get_data_ready_flag()){
			if(Radio.readable()){
				*cmdInOut = '\0';
				char *p_cmdInOut = cmdInOut;
				char temp[50];
				int i;
				
				NRF24L01p::Payload_t payload;
				int error = Radio.ReceivePayload(&payload);
				Radio.clear_data_ready_flag();

				payload.data[payload.length] = '\0';
				*cmdInOut = '\0';
				printf("%d %d %d %s\r\n", error, payload.pipe, payload.length, payload.data);
				sprintf(temp, "%d %d %d \"%s\"\r\n", error, payload.pipe, payload.length, payload.data);
				
				for(i=0;i<strlen(temp);i++){
					*p_cmdInOut = temp[i];
					p_cmdInOut++;				
				}
				while(Radio.readable()){
					error = Radio.ReceivePayload(&payload);
					payload.data[payload.length] = '\0';
					
					printf("%d %d %d %s\r\n", error, payload.pipe, payload.length, payload.data);
					sprintf(temp, "%d %d %d \"%s\"\r\n", error, payload.pipe, payload.length, payload.data);
					for(i=0;i<strlen(temp);i++){
						*p_cmdInOut = temp[i];
						p_cmdInOut++;				
					}
					

				}
				*p_cmdInOut = '\0';
				
			}
		}
	}
}




void RadioTCPBridgeThread(void const *args) {
 printf("Basic HTTP server example\r\n");
    
    EthernetInterface eth;
    eth.connect();
    
    printf("The target IP address is '%s'\r\n", eth.get_ip_address());
    
    TCPServer srv;
    TCPSocket clt_sock;
    SocketAddress clt_addr;
    
    /* Open the server on ethernet stack */
    srv.open(&eth);
    
    /* Bind the HTTP port (TCP 80) to the server */
    srv.bind(eth.get_ip_address(), 80);
    
    /* Can handle 5 simultaneous connections */
    srv.listen(5);
    
    while (true) {
        srv.accept(&clt_sock, &clt_addr);
        printf("accept %s:%d\r\n", clt_addr.get_ip_address(), clt_addr.get_port());
	char recvData[100];
	int len = clt_sock.recv(recvData, 100);
	recvData[len] = '\0';
	printf("DATA[%d] : %s\r\n",len,recvData);
	

	Nrf24l01p_CMD(recvData);

        clt_sock.send(recvData, strlen(recvData));
    }
}
 



// main() runs in its own thread in the OS
int main() {

    Thread thread(RadioTCPBridgeThread);


    while (true) {
        led1 = !led1;
        wait(0.5);
    }
}

