void setup()
{
  Serial.begin(9600);
  pinMode(13,OUTPUT);
}
void loop()
{
 
  if(Serial.available()>0)
  {
    char letter=Serial.read();
  
  if(letter=='1')
  digitalWrite(13,HIGH);
  if(letter=='0')
  digitalWrite(13,LOW);
}
}

