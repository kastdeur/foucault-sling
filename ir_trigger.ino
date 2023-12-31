void ir_interrupt() {
  irState = digitalRead(irPin);
  irMills = millis();
}

void handle_ir_trigger(){
  if ( irMills == 0 ) {
    return;
  }

  // IR sensor reports LOW when seeing something
  if ( irState == LOW )
  {
      irCurrentTriggerStart = irMills;
  }
  else
  {
      irCurrentTriggerEnd = irMills;
  }

  // This interrupt trigger was used
  irMills = 0;
}

void check_ir_trigger() {
  if ( irCurrentTriggerStart == 0 || irCurrentTriggerStart >= irCurrentTriggerEnd )
  { // no period trigger yet
    return;
  }

  #if ENABLE_SERIAL
  Serial.println("--");
  Serial.print("Triggered Period: ");
  Serial.print(curMills - irCurrentTriggerStart);
  Serial.println("ms");

  Serial.print("Last Trigger: ");
  Serial.print(curMills - irPrevTriggerStart);
  Serial.println("ms");
  #endif /* ENABLE_SERIAL */

  // signal this period trigger has been used
  irCurrentTriggerEnd = irCurrentTriggerStart;
  
  // reset trigger counters if we missed a few swings
  if ( irTriggerSetCount != 0 )
  {
    if (irPrevTriggerStart != 0 && curMills - irTriggerSetLast > irTriggerSetInvalidateCount*irTriggerRepeatInterval )
    {
      irTriggerSetCount = 0;
      irTriggerSetCountFinal = irTriggerSetCount;

      #if ENABLE_SERIAL
      Serial.println("Resetting TriggerSet");
      #endif /* ENABLE_SERIAL */
    }
  }

  // do we want to trigger?
  if (curMills - irCurrentTriggerStart <= irTriggerMinPeriod ) {
    #if ENABLE_SERIAL
    Serial.println("Too short Trigger");
    #endif /* ENABLE_SERIAL */
    return;
  }
  else if ( curMills - irCurrentTriggerStart >= irTriggerMaxPeriod
  ) {
    #if ENABLE_SERIAL
    Serial.println("Too long Trigger");
    #endif /* ENABLE_SERIAL */
    return;
  }

  #if ENABLE_SERIAL
  // this seems a valid trigger period length
  Serial.print("Potential Trigger: ");
  #endif /* ENABLE_SERIAL */

  // in shortinterval window?
  if ( curMills - irPrevTriggerStart < irTriggerShortInterval ) {
    // keep the previous triggerstart
    irPrevTriggerStart = irPrevTriggerStart;
    #if ENABLE_SERIAL
    Serial.print("Short Interval");
    #endif /* ENABLE_SERIAL */
  }
  // within repeatInterval from the previous trigger?
  else if (curMills - irPrevTriggerStart >= irTriggerRepeatInterval - irTriggerIntervalLeeway
          && curMills - irPrevTriggerStart <= irTriggerRepeatInterval + irTriggerIntervalLeeway
  ) {
    #if ENABLE_SERIAL
    Serial.print("Repeat Interval");
    #endif /* ENABLE_SERIAL */

    // second trigger of a set?
    if ( curMills -  irTriggerSetLast >= irTriggerSetStaleInterval )
    {
      #if ENABLE_SERIAL
      Serial.println(" (Second of Set)");

      Serial.print("Last Trigger in previous Set: ");
      Serial.print(irTriggerSetLast);
      Serial.println("ms");

      Serial.print("ago: ");
      Serial.println(curMills - irTriggerSetLast);
      Serial.print("Stale at: ");
      Serial.println(irTriggerSetStaleInterval);
      #endif /* ENABLE_SERIAL */

      irTriggerSetStart = irPrevTriggerStart;
      irTriggerSetCount = 0;
      irTriggerSetCountFinal = 0;

      #if ENABLE_SERIAL
      Serial.print("Set Start: ");
      Serial.print(irTriggerSetStart);
      Serial.println("ms");
      #endif /* ENABLE_SERIAL */
    }

    irTriggerSetCount += 1;
    irTriggerSetLast = irCurrentTriggerStart;
    Serial.println();
    Serial.print("TriggerSet Count: ");
    Serial.print(irTriggerSetCount);
  }
  #if ENABLE_SERIAL
  else{
    Serial.print("wait for repeating trigger in ");
    Serial.print(irTriggerRepeatInterval);
    Serial.print(" +- ");
    Serial.print(irTriggerIntervalLeeway);
    Serial.print("ms");
  }

  Serial.println("");
  #endif /* ENABLE_SERIAL */

  irPrevTriggerStart = irCurrentTriggerStart;
}