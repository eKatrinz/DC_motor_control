int b=0;

void setup()
{
  Serial.begin(9600);
  for(byte i=0;i<50;i++)
  {
  b=b+analogRead(0);  
  }
  
  b=b/50;
}
void loop()
{ 
  int c=analogRead(0);
  if(abs(b-c)>30) 
  {
   Serial.println(b-c);
  }
  delay(50);
}
