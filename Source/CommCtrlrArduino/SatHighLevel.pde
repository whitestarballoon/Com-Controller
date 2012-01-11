//Input: pass integer 0 = no parameter setup, 1 = minimal parameter setup, 2 = most parameter setup
// Scratch space: PacketBufferB
void satInitRoutine(byte initParams) {
  unsigned int i;
  byte c=0;
  Serial.println("SatInit");
  digitalWrite(PWR_EN, LOW);   // Turn off the Satcom
  delay(5000);  // Let things settle
  digitalWrite(PWR_EN, HIGH);    // Turn sat power on
  //Initialize the Received Command Numbers Array to zeros
  for (i=EPLOCcmdCounterArrayStart;i<(EPLENcmdCounterArray + EPLOCcmdCounterArrayStart);i++) {
    EEPROM.write(i,0);
  }
  //Delay until satmodem is booted

	// Do or call any sat modem initialization commands required at boot to configure unit for operation

  Serial.println("SatInit Done");
}


void satAOSLOSAlert(){  //Signal Aquisition indicator
  satSyncFlag = digitalRead(sat_sa);   //read sat available pin
  if (satSyncFlag != prevSatSyncStateFlag) {
    if(satSyncFlag == true){
      printf_P(PSTR("AOS\n"));
      lprintf("CC:AOS\n");
    } 
    else {
      printf_P(PSTR("LOS\n"));
      lprintf("CC:LOS\n");
    }
    prevSatSyncStateFlag = satSyncFlag;
  }
}

//Master Outgoing message operations management
//Includes queueing, clearing the queue, monitoring for sent status and prioritization
void satOutgoingMsgOperations(){
  //	Is there sat sync right now?
  //	Are we waiting for uplink commands right now? (i.e. shouldn't be transmitting)
  //Check for Satellite Availability to trigger sending telemetry
  if (LOW == digitalRead(sat_sa)){   //read sat available pin 
	  //Is this a transition to LOW from previous HIGH?
	  if (satSyncFlag) {  //If satSyncFlag is high and the pin is low, we've just transitioned
	  	//Clear long message from sat modem here
	  	satClearOutgoingMHA(satLongMsgInOBQMHA);
	  	satSyncFlag = false);  //set Sync flag to actual state of pin, which is low
	  } 
	  return;  //Leave if there's no signal, nothing left to do.
  } else {
  satSyncFlag = true;
  }
  //Check for being in the delay timer
  if (satWaitingForUplinkCmdsFlag) return;
 
  //Check to see if the satmodem onboard queue is empty or not
  	// SatQ is EMPTY -------------------
  	if (ATCRptReady) {  //Is a new ATC report Ready to send?
  		//Yes there are ATC reports ready to send
  		//LOAD ATC Report pair
  		satSendLatestStoredATCPkt();
  		return;
  	} else {
  		//No ATC report ready to send
  		if(LongMsgReady) {  //Is there a new long Message waiting in the eeprom queue?
  			//YES there is a new long message waiting in the eeprom queue
  			//LOAD next Long message
  			satSendNextQueuedLongMSG(packetBufferS);
  		} else { 
  			//NO there is no long message waiting in the eeprom queue
  			return;
  		}
  	}
  } else {  //SatQ NOT EMPTY -------------------
  	// There are message(s) in the outbound queue
  	if (ATCRptReady) {  //Is a new ATC report Ready to send?
  		satSendLatestStoredATCPkt();
  		return;
  	} else { 
  		//NO there are no ATC reports ready to send
  		//Is existing ATC pair sent from SatQ?
  		satHasATCPairBeenTXed()  // <---  THIS FUNC NOT DONE
  		
  }
  
  
  
  
 